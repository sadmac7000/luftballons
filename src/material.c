/**
 * Copyright © 2013 Casey Dahlin
 *
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Luftballons is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Luftballons.  If not, see <http://www.gnu.org/licenses/>.
 **/ 

#include <err.h>

#include "material.h"
#include "util.h"

/**
 * Deletion backlog. Indicates that there are objects out there which may not
 * have heard about the disappearance of a certain material ID. We track
 * generations so IDs getting thrown out and reallocated doesn't trip us up.
 **/
struct backlog {
	size_t gen_id;
	size_t users_outstanding;
	size_t *releases;
	size_t num_releases;
};

/**
 * Material ID after the highest allocated.
 **/
static size_t alloc_watermark = 0;

/**
 * List of free'd material IDs.
 **/
static size_t *free_ids = NULL;
static size_t num_free_ids = 0;

/**
 * ID of the next backlog generation.
 **/
static size_t gen_id = 0;

/**
 * Number of objects subscribed to the backlog.
 **/
static size_t users_outstanding = 0;

/**
 * List of backlogs.
 **/
static struct backlog *backlogs;
static size_t num_backlogs;

/**
 * Acquire a current generation ID.
 **/
static size_t
material_acquire_gen_id(void)
{
	if (!num_backlogs || backlogs[num_backlogs - 1].num_releases) {
		backlogs = vec_expand(backlogs, num_backlogs);
		backlogs[num_backlogs].gen_id = gen_id++;
		backlogs[num_backlogs].users_outstanding = users_outstanding;
		backlogs[num_backlogs].releases = NULL;
		backlogs[num_backlogs].num_releases = 0;
		num_backlogs++;
	} else {
		backlogs[num_backlogs - 1].users_outstanding++;
	}

	return backlogs[num_backlogs - 1].gen_id;
}

/**
 * Get rid of all no-longer-referenced backlogs.
 **/
static void
material_sweep_backlogs(void)
{
	size_t i;

	for (i = 0; i < num_backlogs;) {
		if (backlogs[i].users_outstanding) {
			i++;
			continue;
		}

		free(backlogs[i].releases);
		backlogs = vec_del(backlogs, num_backlogs, i);
		num_backlogs--;
	}
}

/**
 * Subscribe to the backlogs.
 **/
size_t
material_backlog_subscribe(void)
{
	users_outstanding++;
	return material_acquire_gen_id();
}

/**
 * Unsubscribe from the backlogs.
 **/
void
material_backlog_unsubscribe(size_t gen_id)
{
	size_t i;

	for (i = 0; i < num_backlogs; i++)
		if (backlogs[i].gen_id >= gen_id)
			backlogs[i].users_outstanding--;

	users_outstanding--;
	material_sweep_backlogs();
}

/**
 * Merge a list of releases into the given.
 **/
static void
material_backlog_release_merge(material_t **releases, size_t *num_releases,
			       material_t *other, size_t num_others)
{
	size_t i, j;

	for (i = 0, j = 0; i < *num_releases && j < num_others;) {
		if ((*releases)[i] < other[j]) {
			i++;
			continue;
		}

		if ((*releases)[i] > other[j]) {
			*releases = vec_add(*releases, *num_releases, i, other[j]);
			(*num_releases)++;
		}

		i++;
		j++;
	}

	if (j == num_others)
		return;

	*releases = vec_expand(*releases, *num_releases + num_others - j);
	memcpy(*releases + *num_releases, other + j,
	       (num_others - j) * sizeof(material_t));

	*num_releases += num_others - j;
}

/**
 * Get a list of releases for this backlog, and a new gen_id.
 **/
size_t
material_backlog_sync(size_t gen_id, material_t **releases,
		      size_t *num_releases)
{
	size_t i;

	*releases = NULL;
	*num_releases = 0;

	for (i = 0; i < num_backlogs; i++) {
		if (backlogs[i].gen_id < gen_id)
			continue;

		backlogs[i].users_outstanding--;
		material_backlog_release_merge(releases, num_releases,
					       backlogs[i].releases,
					       backlogs[i].num_releases);
	}

	i = material_acquire_gen_id();
	material_sweep_backlogs();
	return i;
}

/**
 * Allocate a new material ID.
 **/
material_t
material_alloc(void)
{
	size_t ret;

	if (! num_free_ids)
		return alloc_watermark++;

	ret = free_ids[--num_free_ids];
	free_ids = vec_contract(free_ids, num_free_ids);
	return ret;
}
EXPORT(material_alloc);

/**
 * Return whether a material is allocated.
 **/
int
material_is_allocd(material_t mat)
{
	size_t i;

	if (mat == NO_MATERIAL)
		return 0;

	if (mat >= alloc_watermark)
		return 0;

	for (i = 0; i < num_free_ids; i++)
		if (free_ids[i] == mat)
			return 0;

	return 1;
}
EXPORT(material_is_allocd);

/**
 * Deallocate a material ID.
 **/
void
material_destroy(material_t mat)
{
	size_t i, j;

	if (mat == NO_MATERIAL)
		errx(1, "Must pass a material ID to material_destroy");

	free_ids = vec_expand(free_ids, num_free_ids);
	free_ids[num_free_ids++] = mat;

	for (i = 0; i < num_backlogs; i++) {
		for (j = 0; j < backlogs[i].num_releases &&
		     backlogs[i].releases[j] < mat; j++);

		if (backlogs[i].releases[j] == mat)
			continue;

		backlogs[i].releases = vec_add(backlogs[i].releases,
					       backlogs[i].num_releases,
					       j, mat);
		backlogs[i].num_releases++;
	}
}
EXPORT(material_destroy);

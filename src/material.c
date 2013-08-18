/**
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

static size_t alloc_watermark = 0;
static size_t *free_ids = NULL;
static size_t num_free_ids = 0;

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
	if (mat == NO_MATERIAL)
		errx(1, "Must pass a material ID to material_destroy");

	free_ids = vec_expand(free_ids, num_free_ids);
	free_ids[num_free_ids++] = mat;
}
EXPORT(material_destroy);

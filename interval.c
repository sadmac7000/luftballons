/**
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version/
 *
 * Luftballons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Luftballons.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "interval.h"
#include "util.h"

/**
 * Initialize an interval set.
 **/
void
intervals_init(intervals_t *set)
{
	set->by_start = xmalloc(sizeof(intervals_t *));
	set->by_size = xmalloc(sizeof(intervals_t *));
	set->count = 0;
}

/**
 * Free an interval set's resources
 **/
void
intervals_release(intervals_t *set)
{
	size_t i;

	for (i = 0; i < set->count; i++)
		free(set->by_size[i]);

	free(set->by_size);
	free(set->by_start);
}

/**
 * Find an index into the offset-sorted free list of an object that has the
 * given offset. Will return the next consecutive item if there is none.
 **/
static size_t
interval_bisect_start(intervals_t *set, size_t offset)
{
	size_t pos;
	size_t start = 0;
	size_t end = set->count;

	interval_t *cmp;

	while (end != start) {
		pos = (start + end) / 2;
		cmp = set->by_start[pos];

		if (cmp->start < offset && (pos + 1) != end)
			start = pos;
		else if (cmp->start < offset)
			return end;
		else if (cmp->start > offset)
			end = pos;
		else if (cmp->start == offset)
			return pos;
		else if (pos == (end - 1) && end == set->count)
			return set->count;
		else
			return pos;
	}

	return start;
}

/**
 * Find an index into the by_size list of an interval that has the given size.
 * Will return the next consecutive item if there is none.
 **/
static size_t
interval_bisect_size(intervals_t *set, size_t size)
{
	size_t pos;
	size_t start = 0;
	size_t end = set->count;

	interval_t *cmp;

	while (end != start) {
		pos = (start + end) / 2;
		cmp = set->by_size[pos];

		if (cmp->size > size && (pos + 1) != end)
			start = pos;
		else if (cmp->size > size)
			return end;
		else if (cmp->size < size)
			end = pos;
		else if (cmp->size == size)
			return pos;
		else if (pos == (end - 1) && end == set->count)
			return set->count;
		else
			return pos;
	}

	return start;
}

/**
 * Remove an interval from a set that is at the given index in the offset list.
 **/
static void
interval_remove_at_pos(intervals_t *set, size_t offset_idx)
{
	size_t pos;

	interval_t *interval = set->by_start[offset_idx];

	memmove(&set->by_start[offset_idx], &set->by_start[offset_idx + 1],
		(set->count - offset_idx - 1) * sizeof(interval_t *));

	pos = interval_bisect_size(set, interval->size);

	if (set->by_size[pos]->size != interval->size)
		errx(1, "Item missing from interval size list");

	while (pos && set->by_size[pos - 1]->size == interval->size)
		pos--;

	while(set->by_size[pos] != interval)
		pos++;

	memmove(&set->by_size[offset_idx], &set->by_size[offset_idx + 1],
		(set->count - offset_idx - 1) *
		sizeof(interval_t *));

	set->count--;
	free(interval);
}

/**
 * Reposition an item in the size array because it has changed size.
 **/
static void
interval_reposition_by_size(intervals_t *set, size_t idx)
{
	size_t idx_new = idx;
	interval_t *interval = set->by_size[idx];

	while (idx && interval->size < set->by_size[idx - 1]->size)
		idx_new--;

	while (idx < (set->count - 1) &&
	       interval->size < set->by_size[idx + 1]->size)
		idx_new++;

	if (idx_new == idx)
		return;

	if (idx_new > idx)
		memmove(&set->by_size[idx], &set->by_size[idx + 1],
			(idx_new - idx) * sizeof(interval_t *));
	else
		memmove(&set->by_size[idx_new + 1], &set->by_size[idx_new],
			(idx - idx_new) * sizeof(interval_t *));

	set->by_size[idx_new] = interval;
}

/**
 * Reposition an item in the size array because it has changed size.
 **/
static void
interval_handle_resize(intervals_t *set, size_t idx)
{
	size_t idx_new = idx;
	interval_t *interval = set->by_size[idx];

	while (idx && interval->size < set->by_size[idx - 1]->size)
		idx_new--;

	while (idx < (set->count - 1) &&
	       interval->size < set->by_size[idx + 1]->size)
		idx_new++;

	if (idx_new == idx)
		return;

	if (idx_new > idx)
		memmove(&set->by_size[idx], &set->by_size[idx + 1],
			(idx_new - idx) * sizeof(interval_t *));
	else
		memmove(&set->by_size[idx_new + 1], &set->by_size[idx_new],
			(idx - idx_new) * sizeof(interval_t *));

	set->by_size[idx_new] = interval;
}

/**
 * Merge an interval with intervals on either side of it if there is no space
 * between them.
 *
 * set: Interval set to act on.
 * idx: Index in the start list of the interval to merge.
 **/
static void
interval_do_merge(intervals_t *set, size_t idx)
{
	size_t count = 0;
	size_t end_size;
	interval_t *pos = set->by_start[idx];
	interval_t *cmp;

	if (idx + 1 < set->count) {
		cmp = set->by_start[idx + 1];

		if (cmp->start == pos->start + pos->size)
			count++;
	}

	if (idx > 0) {
		cmp = set->by_start[idx - 1];

		if (cmp->start == pos->start + pos->size) {
			count++;
			idx--;
		}
	}

	if (count) {
		cmp = set->by_start[idx + count];
		end_size = cmp->start - pos->start + cmp->size;

		for(;count; count--)
			interval_remove_at_pos(set, idx + count);

		pos->size = end_size;
	}

	interval_handle_resize(set, idx);
}

/**
 * Set an interval in the intervals set.
 **/
void
interval_set(intervals_t *set, size_t start, size_t size)
{
	size_t sz_pos;
	size_t off_pos;

	interval_t *new = xmalloc(sizeof(interval_t));
	interval_t *cmp = NULL;

	new->start = start;
	new->size = size;

	off_pos = interval_bisect_start(set, new->start);

	if (off_pos > 0) {
		cmp = set->by_start[off_pos - 1];

		if (cmp->start + cmp->size == start) {
			cmp->size += size;
			interval_do_merge(set, off_pos - 1);
			return;
		}
	}

	if (off_pos < set->count) {
		cmp = set->by_start[off_pos];

		if (cmp->start == start + size) {
			cmp->start = start;
			cmp->size += size;
			interval_do_merge(set, off_pos);
			return;
		}
	}

	if (cmp) {
		if (cmp->start <= start && cmp->start + cmp->size > start)
			errx(1, "Interval already set");

		if (start <= cmp->start && start + size > cmp->start)
			errx(1, "Interval already set");
	}

	sz_pos = interval_bisect_size(set, new->size);

	set->by_size = vec_expand(set->by_size, set->count);
	set->by_start = vec_expand(set->by_start, set->count);

	memmove(&set->by_size[sz_pos + 1], &set->by_size[sz_pos],
		(set->count - sz_pos) * sizeof(interval_t *));
	memmove(&set->by_start[off_pos + 1], &set->by_start[off_pos],
		(set->count - off_pos) * sizeof(interval_t *));
	set->count++;

	set->by_size[sz_pos] = new;
	set->by_start[off_pos] = new;
}

/**
 * Unset an interval in the intervals set.
 **/
void
interval_unset(intervals_t *set, size_t offset, size_t size)
{
	size_t start = 0;
	size_t end = set->count;
	size_t split;
	size_t create_size = 0;
	interval_t *pos;

	if (! end)
		errx(1, "No intervals set");

	/* We get away with some stuff here because we MUST find a result */
	for (;;) {
		split = (start + end) / 2;
		pos = set->by_start[split];

		if (pos->start > offset) {
			if (! end)
				errx(1, "Interval not set");
			end = split;
		} else if (pos->start + pos->size <= offset) {
			if (start == split)
				errx(1, "Interval not set");
			start = split;
		} else {
			break;
		}
	}

	if ((pos->start + pos->size) > offset + size)
		errx(1, "Interval too large");

	if (pos->start == offset && pos->size == size) {
		interval_remove_at_pos(set, split);
		return;
	}

	if (pos->start == offset) {
		pos->start += size;
		return;
	}

	create_size = pos->size - size - (offset - pos->start);
	pos->size = (offset - pos->start);

	interval_reposition_by_size(set, split);
	if (create_size)
		interval_set(set, pos->start + pos->size + size,
			     create_size);
}

/**
 * Find a set interval of the given size and get its offset.
 *
 * Returns: The offset or -1 if not found.
 **/
ssize_t
interval_find(intervals_t *set, size_t size)
{
	interval_t *pos;

	if (! set->count)
		return -1;

	pos = set->by_size[0];

	if (pos->size < size)
		return -1;

	return pos->start + pos->size - size;
}

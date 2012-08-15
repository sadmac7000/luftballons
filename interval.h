/**
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Luftballons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Luftballons.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef INTERVAL_H
#define INTERVAL_H

/**
 * An interval, which is to say a start and length.
 **/
typedef struct interval {
	size_t start;
	size_t size;
} interval_t;

/**
 * A set of non-overlapping, merged intervals.
 *
 * by_size: List of intervals sorted biggest to smallest.
 * by_start: List of intervals sorted first to last.
 * count: Number of intervals.
 **/
typedef struct intervals {
	interval_t **by_size;
	interval_t **by_start;

	size_t count;
} intervals_t;

#ifdef __cplusplus
extern "C" {
#endif

void intervals_init(intervals_t *set);
void intervals_release(intervals_t *set);
void interval_set(intervals_t *set, size_t start, size_t size);
void interval_unset(intervals_t *set, size_t start, size_t size);
ssize_t interval_find(intervals_t *set, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* INTERVAL_H */

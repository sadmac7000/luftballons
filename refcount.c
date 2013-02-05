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

#include <string.h>

#include "refcount.h"
#include "util.h"

/**
 * Initialize a refcounter. Counter starts set to 0. If it returns to 0 the
 * collection happens. Ungrabbing it while it is at 0 is undefined.
 **/
void
refcount_init(refcounter_t *counter)
{
	memset(counter, 0, sizeof(refcounter_t));
}

/**
 * Add a destructor action to the refcounter. Destructors are run in the
 * reverse of the order they are added.
 **/
void
refcount_add_destructor(refcounter_t *counter, void (*callback)(void *),
			void *data)
{
	refcount_destructor_t *dest;

	vec_expand(counter->destructors, counter->num_destructors);

	dest = &counter->destructors[counter->num_destructors++];
	dest->callback = callback;
	dest->data = data;
}

/**
 * Grab a reference counter.
 **/
void
refcount_grab(refcounter_t *counter)
{
	counter->count++;
}

/**
 * Release a reference counter.
 **/
void
refcount_ungrab(refcounter_t *counter)
{
	size_t i;

	if (counter->count--)
		return;

	for (i = counter->num_destructors; i; i--) {
		counter->destructors[i-1].callback(
			counter->destructors[i-1].data);
	}
}

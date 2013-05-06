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

#ifndef REFCOUNT_H
#define REFCOUNT_H

/**
 * Destructor callback and accompanying data for when a refcount reaches 0.
 **/
typedef struct refcount_destructor {
	void (*callback)(void *);
	void *data;
} refcount_destructor_t;

/**
 * A reference counter.
 **/
typedef struct refcounter {
	size_t count;
	size_t num_destructors;
	refcount_destructor_t *destructors;
} refcounter_t;

#ifdef __cplusplus
extern "C" {
#endif

void refcount_init(refcounter_t *counter);
void refcount_add_destructor(refcounter_t *counter, void (*callback)(void *),
			     void *data);
void refcount_add_destructor_once(refcounter_t *counter,
				  void (*callback)(void *), void *data);
void refcount_grab(refcounter_t *counter);
void refcount_ungrab(refcounter_t *counter);

#ifdef __cplusplus
}
#endif

#endif /* REFCOUNT_H */

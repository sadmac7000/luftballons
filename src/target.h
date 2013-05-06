/**
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Luftballons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Luftballons.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef TARGET_H
#define TARGET_H

#include "state.h"
#include "object.h"
#include "refcount.h"

/**
 * A rendering goal.
 *
 * deps, num_deps: Dependency list for a target.
 * states, num_states: States we need to pass through to hit this target.
 * root: Object we are drawing to reach this target.
 * camera: Camera to draw from.
 * refcount: Reference counter.
 **/
typedef struct target {
	struct target **deps;
	size_t num_deps;

	struct state **states;
	size_t num_states;

	object_t *root;
	object_t *camera;

	refcounter_t refcount;
} target_t;

#ifdef __cplusplus
extern "C" {
#endif

target_t *target_create(object_t *root, object_t *camera);
void target_grab(target_t *target);
void target_ungrab(target_t *target);
void target_add_dep(target_t *target, target_t *dep);
void target_add_state(target_t *target, state_t *state);
void target_hit(target_t *target);

#ifdef __cplusplus
}
#endif

#endif /* TARGET_H */


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
#include <luftballons/target.h>

#include "state.h"
#include "object.h"
#include "refcount.h"
#include "util.h"

/**
 * A rendering goal.
 *
 * deps, num_deps: Dependency list for a target.
 * states, num_states: States we need to pass through to hit this target.
 * camera: Camera to draw from.
 * refcount: Reference counter.
 **/
typedef struct target {
	struct target **deps;
	size_t num_deps;

	state_t **states;
	size_t num_states;

	object_t *camera;

	refcounter_t refcount;
} target_t;

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(target_create);
API_DECLARE(target_grab);
API_DECLARE(target_ungrab);
API_DECLARE(target_add_dep);
API_DECLARE(target_add_state);
API_DECLARE(target_hit);

#ifdef __cplusplus
}
#endif

#endif /* TARGET_H */

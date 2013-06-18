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
#include "colorbuf.h"
#include "draw_op.h"


/**
 * Types of actions taken in a given target step.
 **/
typedef enum target_step_type {
	TARGET_STEP_DRAW,
	TARGET_STEP_TARGET,
	TARGET_STEP_CLEAR,
} target_step_type_t;

/**
 * A step to take in drawing this target.
 **/
struct target;
typedef struct target_step {
	target_step_type_t type;
	union {
		draw_op_t *draw_op;
		struct target *target;
		colorbuf_t *cbuf;
	};
} target_step_t;

/**
 * A rendering goal.
 *
 * base_state: State to push before the individual states.
 * states, num_states: States we need to pass through to hit this target.
 * steps, num_steps: Steps to perform to complete this target.
 * camera: Camera to draw from.
 * repeat: Times to repeat this target's steps.
 * refcount: Reference counter.
 **/
typedef struct target {
	state_t *base_state;

	state_t **states;
	size_t num_states;

	target_step_t *steps;
	size_t num_steps;

	object_t *camera;

	size_t repeat;

	refcounter_t refcount;
} target_t;

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(target_create);
API_DECLARE(target_grab);
API_DECLARE(target_ungrab);
API_DECLARE(target_add_state);
API_DECLARE(target_clear);
API_DECLARE(target_draw);
API_DECLARE(target_hit);
API_DECLARE(target_hit_other);

#ifdef __cplusplus
}
#endif

#endif /* TARGET_H */

/**
 * Copyright © 2013 Casey Dahlin
 *
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

#ifndef DRAW_PROC_H
#define DRAW_PROC_H
#include <luftballons/draw_proc.h>

#include "state.h"
#include "object.h"
#include "refcount.h"
#include "util.h"
#include "colorbuf.h"
#include "draw_op.h"
#include "shader.h"
#include "material.h"


/**
 * Types of actions taken in a given draw_proc step.
 **/
typedef enum draw_proc_step_type {
	DRAW_PROC_STEP_DRAW,
	DRAW_PROC_STEP_PROC,
	DRAW_PROC_STEP_CLEAR,
} draw_proc_step_type_t;

/**
 * A step to take in drawing this draw_proc.
 **/
struct draw_proc;
typedef struct draw_proc_step {
	draw_proc_step_type_t type;
	union {
		draw_op_t *draw_op;
		struct draw_proc *draw_proc;
		colorbuf_t *cbuf;
	};
} draw_proc_step_t;

/**
 * A rendering goal.
 *
 * base_state: State to push before the individual states.
 * steps, num_steps: Steps to perform to complete this draw_proc.
 * repeat: Times to repeat this draw_proc's steps.
 * refcount: Reference counter.
 **/
typedef struct draw_proc {
	state_t *base_state;

	draw_proc_step_t *steps;
	size_t num_steps;

	size_t repeat;

	refcounter_t refcount;
} draw_proc_t;

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(draw_proc_create);
API_DECLARE(draw_proc_clone);
API_DECLARE(draw_proc_grab);
API_DECLARE(draw_proc_ungrab);
API_DECLARE(draw_proc_clear);
API_DECLARE(draw_proc_draw);
API_DECLARE(draw_proc_run);
API_DECLARE(draw_proc_run_other);
API_DECLARE(draw_proc_set_shader);
API_DECLARE(draw_proc_set_blend);
API_DECLARE(draw_proc_set_flags);
API_DECLARE(draw_proc_clear_flags);
API_DECLARE(draw_proc_ignore_flags);
API_DECLARE(draw_proc_set_colorbuf);
API_DECLARE(draw_proc_set_uniform);

#ifdef __cplusplus
}
#endif

#endif /* DRAW_PROC_H */

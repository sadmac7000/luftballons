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

#ifndef DRAW_QUEUE_H
#define DRAW_QUEUE_H

#include "object.h"
#include "util.h"
#include "state.h"
#include "refcount.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A draw operation.
 * 
 * object: Object root to draw.
 * camera: Camera to draw from.
 * state: State to enter while drawing.
 * refcount: Reference count for this object.
 **/
typedef struct draw_op {
	object_t *object;
	object_t *camera;
	state_t *state;

	refcounter_t refcount;
} draw_op_t;

draw_op_t *draw_op_create(object_t *object, object_t *camera, state_t *state);
void draw_op_grab(draw_op_t *op);
void draw_op_ungrab(draw_op_t *op);
void draw_op_exec(draw_op_t *op);

#ifdef __cplusplus
}
#endif

#endif /* DRAW_QUEUE_H */

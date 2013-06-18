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

#include "target.h"
#include "util.h"
#include "draw_op.h"

/**
 * The destructor for a target_t
 **/
static void
target_destructor(void *target_)
{
	target_t *target = target_;
	size_t i;

	for (i = 0; i < target->num_steps; i++) {
		if (target->steps[i].type == TARGET_STEP_DRAW)
			draw_op_ungrab(target->steps[i].draw_op);
		else if (target->steps[i].type == TARGET_STEP_TARGET)
			target_ungrab(target->steps[i].target);
		else if (target->steps[i].type == TARGET_STEP_CLEAR)
			colorbuf_ungrab(target->steps[i].cbuf);
		else
			errx(1, "Encountered target step with unknown type");
	}

	if (target->base_state)
		state_ungrab(target->base_state);

	free(target->steps);
	free(target);
}

/**
 * Create a new target.
 *
 * base: The base state for this target.
 * repeat: Repeat count for this target.
 **/
target_t *
target_create(state_t *base, size_t repeat)
{
	target_t *ret = xcalloc(1, sizeof(target_t));

	ret->base_state = base;

	if (base)
		state_grab(base);

	ret->repeat = repeat;

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, target_destructor, ret);

	return ret;
}
EXPORT(target_create);

/**
 * Grab a target.
 **/
void
target_grab(target_t *target)
{
	refcount_grab(&target->refcount);
}
EXPORT(target_grab);

/**
 * Ungrab a target.
 **/
void
target_ungrab(target_t *target)
{
	refcount_ungrab(&target->refcount);
}
EXPORT(target_ungrab);

/**
 * Add a draw op to our list of steps.
 **/
void
target_draw(target_t *target, draw_op_t *op)
{
	target->steps = vec_expand(target->steps, target->num_steps);

	target->steps[target->num_steps].type = TARGET_STEP_DRAW;
	target->steps[target->num_steps].draw_op = op;
	target->num_steps++;

	draw_op_grab(op);
}
EXPORT(target_draw);

/**
 * Add another target to our list of steps.
 **/
void
target_hit_other(target_t *target, target_t *other)
{
	target->steps = vec_expand(target->steps, target->num_steps);

	target->steps[target->num_steps].type = TARGET_STEP_TARGET;
	target->steps[target->num_steps].target = other;
	target->num_steps++;

	target_grab(other);
}
EXPORT(target_hit_other);

/**
 * Set a colorbuf to be cleared by this target after its dependencies are
 * satisfied, but before it runs its draw operations.
 **/
void
target_clear(target_t *target, colorbuf_t *buf)
{
	target->steps = vec_expand(target->steps, target->num_steps);

	target->steps[target->num_steps].type = TARGET_STEP_CLEAR;
	target->steps[target->num_steps].cbuf = buf;
	target->num_steps++;

	colorbuf_grab(buf);
}
EXPORT(target_clear);

/**
 * Add a state that is passed through in order to hit a target.
 **/
void
target_draw_state(target_t *target, state_t *state,
		  object_t *camera, object_t *root)
{
	draw_op_t *op = draw_op_create(root, camera, state);

	target_draw(target, op);
	draw_op_ungrab(op);
}
EXPORT(target_draw_state);

/**
 * Perform one step of a target.
 **/
static void
target_do_step(target_step_t *step)
{
	if (step->type == TARGET_STEP_DRAW)
		draw_op_exec(step->draw_op);
	else if (step->type == TARGET_STEP_TARGET)
		target_hit(step->target);
	else if (step->type == TARGET_STEP_CLEAR)
		colorbuf_clear(step->cbuf);
	else
		errx(1, "Encountered target step with unknown type");
}

/**
 * Hit this target exactly once.
 **/
static void
target_hit_once(target_t *target)
{
	size_t i;

	if (target->base_state)
		state_push(target->base_state);

	for (i = 0; i < target->num_steps; i++)
		target_do_step(&target->steps[i]);

	if (target->base_state)
		state_pop(target->base_state);
}

/**
 * Hit this target
 **/
void
target_hit(target_t *target)
{
	size_t count = target->repeat;
	size_t i;

	for (i = 0; i < count; i++)
		target_hit_once(target);
}
EXPORT(target_hit);

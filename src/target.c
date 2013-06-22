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

#include <stdarg.h>

#include "target.h"
#include "util.h"
#include "draw_op.h"
#include "uniform.h"

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
 * repeat: Repeat count for this target.
 **/
target_t *
target_create(size_t repeat)
{
	target_t *ret = xcalloc(1, sizeof(target_t));

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

/**
 * Ensure this target has a base state.
 **/
static void
target_init_state(target_t *target)
{
	if (! target->base_state)
		target->base_state = state_create(NULL);
}

/**
 * Set a shader to use for this target when not overriden.
 **/
void
target_set_shader(target_t *target, shader_t *shader)
{
	target_init_state(target);
	state_set_shader(target->base_state, shader);
}
EXPORT(target_set_shader);

/**
 * Set a blend mode for this target.
 **/
void
target_set_blend(target_t *target, state_blend_mode_t mode)
{
	target_init_state(target);
	state_set_blend(target->base_state, mode);
}
EXPORT(target_set_blend);

/**
 * Set flags this target would like to be enabled.
 **/
void
target_set_flags(target_t *target, uint64_t flags)
{
	target_init_state(target);
	state_set_flags(target->base_state, flags);
}
EXPORT(target_set_flags);

/**
 * Set flags this target would like disabled.
 **/
void
target_clear_flags(target_t *target, uint64_t flags)
{
	target_init_state(target);
	state_clear_flags(target->base_state, flags);
}
EXPORT(target_clear_flags);

/**
 * Set flags this target doesn't care about.
 **/
void
target_ignore_flags(target_t *target, uint64_t flags)
{
	target_init_state(target);
	state_ignore_flags(target->base_state, flags);
}
EXPORT(target_ignore_flags);

/**
 * Set the colorbuf for this target.
 **/
void
target_set_colorbuf(target_t *target, colorbuf_t *colorbuf)
{
	target_init_state(target);
	state_set_colorbuf(target->base_state, colorbuf);
}
EXPORT(target_set_colorbuf);

/**
 * Set a uniform for this target.
 **/
void
target_set_uniform(target_t *target, uniform_type_t type, ...)
{
	va_list ap;
	uniform_t *uniform;

	va_start(ap, type);
	uniform = uniform_vcreate(type, ap);
	va_end(ap);

	target_init_state(target);
	state_set_uniform(target->base_state, LUFT_UNIFORM_CLONE, uniform);
	uniform_ungrab(uniform);
}
EXPORT(target_set_uniform);

/**
 * Set the material to draw for this target.
 **/
void
target_set_material(target_t *target, int mat_id)
{
	target_init_state(target);
	state_set_material(target->base_state, mat_id);
}
EXPORT(target_set_material);

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

#include "draw_proc.h"
#include "util.h"
#include "draw_op.h"
#include "uniform.h"

/**
 * The destructor for a draw_proc_t
 **/
static void
draw_proc_destructor(void *draw_proc_)
{
	draw_proc_t *draw_proc = draw_proc_;
	size_t i;

	for (i = 0; i < draw_proc->num_steps; i++) {
		if (draw_proc->steps[i].type == TARGET_STEP_DRAW)
			draw_op_ungrab(draw_proc->steps[i].draw_op);
		else if (draw_proc->steps[i].type == TARGET_STEP_TARGET)
			draw_proc_ungrab(draw_proc->steps[i].draw_proc);
		else if (draw_proc->steps[i].type == TARGET_STEP_CLEAR)
			colorbuf_ungrab(draw_proc->steps[i].cbuf);
		else
			errx(1, "Encountered draw_proc step with unknown type");
	}

	if (draw_proc->base_state)
		state_ungrab(draw_proc->base_state);

	free(draw_proc->steps);
	free(draw_proc);
}

/**
 * Create a new draw procedure.
 *
 * repeat: Times to repeat the operations in this procedure.
 **/
draw_proc_t *
draw_proc_create(size_t repeat)
{
	draw_proc_t *ret = xcalloc(1, sizeof(draw_proc_t));

	ret->repeat = repeat;

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, draw_proc_destructor, ret);

	return ret;
}
EXPORT(draw_proc_create);

/**
 * Clone an existing draw procedure.
 **/
draw_proc_t *
draw_proc_clone(draw_proc_t *draw_proc)
{
	draw_proc_t *ret = xmemdup(draw_proc, sizeof(draw_proc_t));
	size_t i;

	if (ret->base_state)
		ret->base_state = state_clone(ret->base_state);

	ret->steps = vec_expand((draw_proc_step_t *)NULL, ret->num_steps);
	memcpy(ret->steps, draw_proc->steps,
	       ret->num_steps * sizeof(draw_proc_step_t));

	for (i = 0; i < ret->num_steps; i++) {
		if (ret->steps[i].type == TARGET_STEP_DRAW)
			draw_op_grab(ret->steps[i].draw_op);
		else if (ret->steps[i].type == TARGET_STEP_TARGET)
			draw_proc_grab(ret->steps[i].draw_proc);
		else if (ret->steps[i].type == TARGET_STEP_CLEAR)
			colorbuf_grab(ret->steps[i].cbuf);
		else
			errx(1, "Draw procedure steps must "
			     "have recognized types");
	}

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, draw_proc_destructor, ret);

	return ret;
}
EXPORT(draw_proc_clone);

/**
 * Grab a draw_proc.
 **/
void
draw_proc_grab(draw_proc_t *draw_proc)
{
	refcount_grab(&draw_proc->refcount);
}
EXPORT(draw_proc_grab);

/**
 * Ungrab a draw_proc.
 **/
void
draw_proc_ungrab(draw_proc_t *draw_proc)
{
	refcount_ungrab(&draw_proc->refcount);
}
EXPORT(draw_proc_ungrab);

/**
 * Add a draw op to our list of steps.
 **/
void
draw_proc_draw(draw_proc_t *draw_proc, draw_op_t *op)
{
	draw_proc->steps = vec_expand(draw_proc->steps, draw_proc->num_steps);

	draw_proc->steps[draw_proc->num_steps].type = TARGET_STEP_DRAW;
	draw_proc->steps[draw_proc->num_steps].draw_op = op;
	draw_proc->num_steps++;

	draw_op_grab(op);
}
EXPORT(draw_proc_draw);

/**
 * Add another draw_proc to our list of steps.
 **/
void
draw_proc_hit_other(draw_proc_t *draw_proc, draw_proc_t *other)
{
	draw_proc->steps = vec_expand(draw_proc->steps, draw_proc->num_steps);

	draw_proc->steps[draw_proc->num_steps].type = TARGET_STEP_TARGET;
	draw_proc->steps[draw_proc->num_steps].draw_proc = other;
	draw_proc->num_steps++;

	draw_proc_grab(other);
}
EXPORT(draw_proc_hit_other);

/**
 * Set a colorbuf to be cleared by this draw_proc after its dependencies are
 * satisfied, but before it runs its draw operations.
 **/
void
draw_proc_clear(draw_proc_t *draw_proc, colorbuf_t *buf)
{
	draw_proc->steps = vec_expand(draw_proc->steps, draw_proc->num_steps);

	draw_proc->steps[draw_proc->num_steps].type = TARGET_STEP_CLEAR;
	draw_proc->steps[draw_proc->num_steps].cbuf = buf;
	draw_proc->num_steps++;

	colorbuf_grab(buf);
}
EXPORT(draw_proc_clear);

/**
 * Perform one step of a draw_proc.
 **/
static void
draw_proc_do_step(draw_proc_step_t *step)
{
	if (step->type == TARGET_STEP_DRAW)
		draw_op_exec(step->draw_op);
	else if (step->type == TARGET_STEP_TARGET)
		draw_proc_hit(step->draw_proc);
	else if (step->type == TARGET_STEP_CLEAR)
		colorbuf_clear(step->cbuf);
	else
		errx(1, "Encountered draw_proc step with unknown type");
}

/**
 * Hit this draw_proc exactly once.
 **/
static void
draw_proc_hit_once(draw_proc_t *draw_proc)
{
	size_t i;

	if (draw_proc->base_state)
		state_push(draw_proc->base_state);

	for (i = 0; i < draw_proc->num_steps; i++)
		draw_proc_do_step(&draw_proc->steps[i]);

	if (draw_proc->base_state)
		state_pop(draw_proc->base_state);
}

/**
 * Hit this draw_proc
 **/
void
draw_proc_hit(draw_proc_t *draw_proc)
{
	size_t count = draw_proc->repeat;
	size_t i;

	for (i = 0; i < count; i++)
		draw_proc_hit_once(draw_proc);
}
EXPORT(draw_proc_hit);

/**
 * Ensure this draw_proc has a base state.
 **/
static void
draw_proc_init_state(draw_proc_t *draw_proc)
{
	if (! draw_proc->base_state)
		draw_proc->base_state = state_create(NULL);
}

/**
 * Set a shader to use for this draw_proc when not overriden.
 **/
void
draw_proc_set_shader(draw_proc_t *draw_proc, shader_t *shader)
{
	draw_proc_init_state(draw_proc);
	state_set_shader(draw_proc->base_state, shader);
}
EXPORT(draw_proc_set_shader);

/**
 * Set a blend mode for this draw_proc.
 **/
void
draw_proc_set_blend(draw_proc_t *draw_proc, state_blend_mode_t mode)
{
	draw_proc_init_state(draw_proc);
	state_set_blend(draw_proc->base_state, mode);
}
EXPORT(draw_proc_set_blend);

/**
 * Set flags this draw_proc would like to be enabled.
 **/
void
draw_proc_set_flags(draw_proc_t *draw_proc, uint64_t flags)
{
	draw_proc_init_state(draw_proc);
	state_set_flags(draw_proc->base_state, flags);
}
EXPORT(draw_proc_set_flags);

/**
 * Set flags this draw_proc would like disabled.
 **/
void
draw_proc_clear_flags(draw_proc_t *draw_proc, uint64_t flags)
{
	draw_proc_init_state(draw_proc);
	state_clear_flags(draw_proc->base_state, flags);
}
EXPORT(draw_proc_clear_flags);

/**
 * Set flags this draw_proc doesn't care about.
 **/
void
draw_proc_ignore_flags(draw_proc_t *draw_proc, uint64_t flags)
{
	draw_proc_init_state(draw_proc);
	state_ignore_flags(draw_proc->base_state, flags);
}
EXPORT(draw_proc_ignore_flags);

/**
 * Set the colorbuf for this draw_proc.
 **/
void
draw_proc_set_colorbuf(draw_proc_t *draw_proc, colorbuf_t *colorbuf)
{
	draw_proc_init_state(draw_proc);
	state_set_colorbuf(draw_proc->base_state, colorbuf);
}
EXPORT(draw_proc_set_colorbuf);

/**
 * Set a uniform for this draw_proc.
 **/
void
draw_proc_set_uniform(draw_proc_t *draw_proc, uniform_type_t type, ...)
{
	va_list ap;
	uniform_t *uniform;

	va_start(ap, type);
	uniform = uniform_vcreate(type, ap);
	va_end(ap);

	draw_proc_init_state(draw_proc);
	state_set_uniform(draw_proc->base_state, LUFT_UNIFORM_CLONE, uniform);
	uniform_ungrab(uniform);
}
EXPORT(draw_proc_set_uniform);

/**
 * Set the material to draw for this draw_proc.
 **/
void
draw_proc_set_material(draw_proc_t *draw_proc, int mat_id)
{
	draw_proc_init_state(draw_proc);
	state_set_material(draw_proc->base_state, mat_id);
}
EXPORT(draw_proc_set_material);

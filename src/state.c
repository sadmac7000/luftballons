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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <GL/gl.h>

#include "state.h"
#include "util.h"

/**
 * The current state.
 **/
state_t *current_state = NULL;

/**
 * The current state stack.
 **/
state_t **state_stack;
size_t state_stack_size = 0;

/**
 * Destroy a state object. Don't defer until the state is no longer active.
 **/
static void
state_destructor(void *data)
{
	size_t i;
	state_t *state = data;

	for (i = 0; i < state->num_uniforms; i++)
		uniform_ungrab(state->uniforms[i]);

	if (state->colorbuf)
		colorbuf_ungrab(state->colorbuf);


	free(state->uniforms);
	free(state);
}

/**
 * Create a new state object.
 **/
state_t *
state_create(shader_t *shader)
{
	state_t *state = xcalloc(1, sizeof(state_t));

	state_set_shader(state, shader);
	state->mat_id = -1;
	state->blend_mode = STATE_BLEND_DONTCARE;

	refcount_init(&state->refcount);
	refcount_add_destructor(&state->refcount, state_destructor, state);

	return state;
}
EXPORT(state_create);

/**
 * Set a state object's shader.
 **/
void
state_set_shader(state_t *state, shader_t *shader)
{
	if (state->shader)
		shader_ungrab(state->shader);

	if (shader)
		shader_grab(shader);

	state->shader = shader;
}
EXPORT(state_set_shader);

/**
 * Create a new state with the same properties as an existing state.
 **/
state_t *
state_clone(state_t *in)
{
	state_t *state = xmalloc(sizeof(state_t));
	size_t i;

	memcpy(state, in, sizeof(state_t));

	if (state->root)
		object_grab(state->root);

	if (state->colorbuf)
		colorbuf_grab(state->colorbuf);

	for (i = 0; i < state->num_uniforms; i++)
		uniform_grab(state->uniforms[i]);

	refcount_init(&state->refcount);
	refcount_add_destructor(&state->refcount, state_destructor, state);

	return state;
}
EXPORT(state_clone);

/**
 * Grab a state object.
 **/
void
state_grab(state_t *state)
{
	refcount_grab(&state->refcount);
}
EXPORT(state_grab);

/**
 * Ungrab a state object.
 **/
void
state_ungrab(state_t *state)
{
	refcount_ungrab(&state->refcount);
}
EXPORT(state_ungrab);

/**
 * Set up depth testing as this state states it should be.
 **/
static void
state_set_depth_test(state_t *state)
{
	if (state->flags & STATE_DEPTH_TEST)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	CHECK_GL;
}

/**
 * Set up backface culling as this state states it should be.
 **/
static void
state_set_bf_cull(state_t *state)
{
	if (state->flags & STATE_BF_CULL) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	} else {
		glDisable(GL_CULL_FACE);
	}

	CHECK_GL;
}

/**
 * Apply this state's blend mode to OpenGL.
 **/
static void
state_apply_blend_mode(state_t *state, state_blend_mode_t old)
{
	if (state->blend_mode == STATE_BLEND_DONTCARE)
		return;

	if (state->blend_mode == old)
		return;

	if (state->blend_mode == STATE_BLEND_NONE) {
		glDisable(GL_BLEND);
		return;
	}

	if (old == STATE_BLEND_NONE || old == STATE_BLEND_DONTCARE)
		glEnable(GL_BLEND);

	if (state->blend_mode == STATE_BLEND_ALPHA)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else if (state->blend_mode == STATE_BLEND_ADDITIVE)
		glBlendFunc(GL_ONE, GL_ONE);
	else if (state->blend_mode == STATE_BLEND_REVERSE_ALPHA)
		glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
}


/**
 * Enter the given state.
 **/
static void
state_enter(state_t *state)
{
	uint64_t change_flags = state->care_about;
	size_t i;
	state_blend_mode_t old = STATE_BLEND_DONTCARE;

	if (current_state == state)
		return;

	state_grab(state);

	for (i = 0; i < state->num_uniforms; i++)
		shader_set_uniform(state->shader, state->uniforms[i]);

	shader_activate(state->shader);

	if (current_state) {
		change_flags = current_state->flags ^ state->flags;
		change_flags |= ~current_state->care_about;
		change_flags &= state->care_about;
		old = current_state->blend_mode;
	}

	if (change_flags & STATE_DEPTH_TEST)
		state_set_depth_test(state);

	if (change_flags & STATE_BF_CULL)
		state_set_bf_cull(state);
	
	state_apply_blend_mode(state, old);
	colorbuf_prep(state->colorbuf);

	if (current_state)
		state_ungrab(current_state);

	current_state = state;
}

/**
 * Set all values that we don't care about in the given state to the values
 * given by other, where other does care about those values.
 **/
static void
state_underlay(state_t *state, state_t *other)
{
	uint64_t set;
	uint64_t clear;
	size_t i;
	size_t j;

	set = other->flags;
	clear = ~other->flags;

	set &= other->care_about;
	clear &= other->care_about;

	set &= ~state->care_about;
	clear &= ~state->care_about;

	state->care_about |= set | clear;
	state->flags |= set;
	state->flags &= ~clear;

	if (state->mat_id == -1)
		state->mat_id = other->mat_id;

	if (!state->colorbuf && other->colorbuf) {
		state->colorbuf = other->colorbuf;
		colorbuf_grab(other->colorbuf);
	}

	if (!state->shader && other->shader) {
		state->shader = other->shader;
		shader_grab(other->shader);
	}

	if (!state->root && other->root) {
		state->root = other->root;
		object_grab(other->root);
	}

	if (state->blend_mode == STATE_BLEND_DONTCARE)
		state->blend_mode = other->blend_mode;

	for (i = 0; i < other->num_uniforms; i++) {
		for (j = 0; j < state->num_uniforms; j++)
			if (! strcmp(state->uniforms[j]->name,
				     other->uniforms[i]->name))
				break;

		if (j == state->num_uniforms)
			state_set_uniform(state, UNIFORM_CLONE,
					  other->uniforms[i]);
	}
}

/**
 * Compile the state stack into a single state and enter it.
 **/
static void
state_stack_aggregate(void)
{
	state_t *state;
	size_t i;

	if (! state_stack_size)
		return;

	if (state_stack_size == 1) {
		state_enter(state_stack[0]);
		return;
	}

	state = state_create(NULL);

	for (i = state_stack_size; i; i--)
		state_underlay(state, state_stack[i - 1]);

	state_enter(state);
	state_ungrab(state);
}

/**
 * Push a state on to the stack.
 **/
void
state_push(state_t *state)
{
	state_stack = vec_expand(state_stack, state_stack_size);

	state_stack[state_stack_size++] = state;
	state_grab(state);

	state_stack_aggregate();
}

/**
 * Pop a state from the stack.
 *
 * state: If the state popped isn't equal to this value, and this value isn't
 *        NULL, raise an error
 **/
void
state_pop(state_t *state)
{
	state_t *popped;
	if (! state_stack_size) {
		if (state)
			errx(1, "Expected to pop a state when"
			     " there was none pushed");

		return;
	}

	popped = state_stack[--state_stack_size];

	if (state && popped != state)
		errx(1, "Unexpected state popped");

	state_ungrab(popped);

	state_stack_aggregate();
}

/**
 * Protect the state in case it is the current state and we need to modify it.
 **/
static void
state_protect(state_t *state, int *needs_it)
{
	state_t *tmp;

	*needs_it = (state == current_state);

	if (! *needs_it)
		return;

	state_grab(state);
	tmp = state_clone(state);
	state_enter(tmp);
	state_ungrab(tmp);
}

/**
 * Undo a state_protect call.
 **/
static void
state_unprotect(state_t *state, int needed_it)
{
	if (! needed_it)
		return;

	state_enter(state);
	state_ungrab(state);
}

/**
 * Set the given flags in the given state.
 **/
void
state_set_flags(state_t *state, uint64_t flags)
{
	int needed_it;

	state_protect(state, &needed_it);

	state->flags |= flags;
	state->care_about |= flags;

	state_unprotect(state, needed_it);
}
EXPORT(state_set_flags);

/**
 * Clear the given flags in the given state.
 **/
void
state_clear_flags(state_t *state, uint64_t flags)
{
	int needed_it;

	state_protect(state, &needed_it);

	state->flags &= ~flags;
	state->care_about |= flags;

	state_unprotect(state, needed_it);
}
EXPORT(state_clear_flags);

/**
 * Indicate that in the given state, the given flags may hold any value.
 **/
void
state_ignore_flags(state_t *state, uint64_t flags)
{
	int needed_it;

	state_protect(state, &needed_it);

	state->care_about &= ~flags;

	state_unprotect(state, needed_it);
}
EXPORT(state_ignore_flags);

/**
 * Set the output color buffer. Set NULL to use the default framebuffer.
 **/
void
state_set_colorbuf(state_t *state, colorbuf_t *colorbuf)
{
	int needed_it;

	state_protect(state, &needed_it);

	if (state->colorbuf)
		colorbuf_ungrab(state->colorbuf);

	colorbuf_grab(colorbuf);
	state->colorbuf = colorbuf;

	state_unprotect(state, needed_it);
}
EXPORT(state_set_colorbuf);

/**
 * Set a uniform that is applied when this state is entered.
 **/
void
state_set_uniform(state_t *state, uniform_type_t type, ...)
{
	size_t i;
	uniform_t *uniform;
	va_list ap;

	va_start(ap, type);

	uniform = uniform_vcreate(type, ap);

	va_end(ap);

	for (i = 0; i < state->num_uniforms; i++) {
		if (strcmp(state->uniforms[i]->name, uniform->name))
			continue;

		uniform_ungrab(state->uniforms[i]);
		state->uniforms[i] = uniform;
		return;
	}

	state->uniforms = vec_expand(state->uniforms, state->num_uniforms);
	state->uniforms[state->num_uniforms++] = uniform;

	if (state == current_state)
		shader_set_uniform(state->shader, uniform);
}
EXPORT(state_set_uniform);

/**
 * Set state material ID.
 **/
void
state_set_material(state_t *state, int mat_id)
{
	state->mat_id = mat_id;
}
EXPORT(state_set_material);

/**
 * Determine if a given material is active in the current state.
 **/
int
state_material_active(int mat_id)
{
	if (! current_state)
		return 0;

	if (current_state->mat_id < 0)
		return 1;

	return current_state->mat_id == mat_id;
}

/**
 * Set the object to draw in this state.
 **/
void
state_set_object(state_t *state, object_t *object)
{
	int needed_it;

	state_protect(state, &needed_it);

	if (state->root)
		object_ungrab(state->root);

	if (object)
		object_grab(object);

	state->root = object;

	state_unprotect(state, needed_it);
}
EXPORT(state_set_object);

/**
 * Set the blending mode for this state.
 **/
void
state_set_blend(state_t *state, state_blend_mode_t mode)
{
	int needed_it;

	state_protect(state, &needed_it);

	state->blend_mode = mode;

	state_unprotect(state, needed_it);
}
EXPORT(state_set_blend);

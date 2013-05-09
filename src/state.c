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

#include <GL/gl.h>

#include "state.h"
#include "util.h"

/**
 * The current state.
 **/
state_t *current_state = NULL;

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

	state->shader = shader;
	state->mat_id = -1;

	refcount_init(&state->refcount);
	refcount_add_destructor(&state->refcount, state_destructor, state);

	return state;
}
EXPORT(state_create);

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
 * Set up alpha blending as this state states it should be.
 **/
static void
state_set_alpha_blend(state_t *state)
{
	if (state->flags & STATE_ALPHA_BLEND) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}

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
 * Enter the given state.
 **/
void
state_enter(state_t *state)
{
	uint64_t change_flags = state->care_about;
	size_t i;

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
	}

	if (change_flags & STATE_DEPTH_TEST)
		state_set_depth_test(state);

	if (change_flags & STATE_ALPHA_BLEND)
		state_set_alpha_blend(state);

	if (change_flags & STATE_BF_CULL)
		state_set_bf_cull(state);
	
	colorbuf_prep(state->colorbuf);

	if (current_state)
		state_ungrab(current_state);

	current_state = state;
}

/**
 * Crash if this state is the current state.
 **/
static void
state_assert_not_current(state_t *state)
{
	if (state == current_state)
		errx(1, "Attempt to modify active state");
}

/**
 * Set the given flags in the given state.
 **/
void
state_set_flags(state_t *state, uint64_t flags)
{
	state_assert_not_current(state);

	state->flags |= flags;
	state->care_about |= flags;
}
EXPORT(state_set_flags);

/**
 * Clear the given flags in the given state.
 **/
void
state_clear_flags(state_t *state, uint64_t flags)
{
	state_assert_not_current(state);

	state->flags &= ~flags;
	state->care_about |= flags;
}
EXPORT(state_clear_flags);

/**
 * Indicate that in the given state, the given flags may hold any value.
 **/
void
state_ignore_flags(state_t *state, uint64_t flags)
{
	state_assert_not_current(state);

	state->care_about &= ~flags;
}
EXPORT(state_ignore_flags);

/**
 * Set the output color buffer. Set NULL to use the default framebuffer.
 **/
void
state_set_colorbuf(state_t *state, colorbuf_t *colorbuf)
{
	state_assert_not_current(state);

	if (state->colorbuf)
		colorbuf_ungrab(state->colorbuf);

	colorbuf_grab(colorbuf);
	state->colorbuf = colorbuf;
}
EXPORT(state_set_colorbuf);

/**
 * Set a uniform that is applied when this state is entered.
 **/
void
state_set_uniform(state_t *state, uniform_t *uniform)
{
	size_t i;

	uniform_grab(uniform);

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
	if (state->root)
		object_ungrab(state->root);

	if (object)
		object_grab(object);

	state->root = object;
}
EXPORT(state_set_object);

/**
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Luftballons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
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

GLuint framebuf;
texmap_t **framebuf_maps = NULL;
size_t framebuf_maps_size = 0;

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

	for (i = 0; i < state->num_dependants; i++)
		state_ungrab(state->dependants[i]);

	for(i = 0; i < state->num_colorbufs; i++)
		texmap_ungrab(state->colorbufs[i]);

	free(state->colorbufs);
	free(state->uniforms);
	free(state->dependants);
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

/**
 * Grab a state object.
 **/
void
state_grab(state_t *state)
{
	refcount_grab(&state->refcount);
}

/**
 * Ungrab a state object.
 **/
void
state_ungrab(state_t *state)
{
	refcount_ungrab(&state->refcount);
}

/**
 * Mark that a state depends on another state.
 **/
void
state_depends_on(state_t *state, state_t *other)
{
	other->dependants = vec_expand(other->dependants,
				       other->num_dependants);

	other->dependants[other->num_dependants++] = state;

	state_grab(state);
}

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
}

/**
 * Set up 2D texture support as this state states it should be.
 **/
static void
state_set_texture_2D(state_t *state)
{
	if (state->flags & STATE_TEXTURE_2D)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);
}

/**
 * Prepare to run on the default buffer.
 **/
static void
state_prep_default_colorbufs(void)
{
	if (current_state && current_state->num_colorbufs == 0)
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
}

/**
 * Allocate a color attachment in the framebuffer.
 **/
static int
state_alloc_framebuffer(texmap_t *map, GLenum *attach)
{
	size_t i;

	/* FIXME: If malloc reuses a memory segment we could destroy a texmap
	 * and then have a new one with the same address, which would appear to
	 * be in the framebuffer when it isn't.
	 *
	 * We can get away with freed textures that aren't reallocated because
	 * we only compare pointers, we don't dereference.
	 */
	for (i = 0; i < framebuf_maps_size; i++) {
		if (framebuf_maps[i] == map)
			break;
	}

	if (i == framebuf_maps_size) {
		if (framebuf_maps_size == state_max_colorbufs())
			return -1;

		framebuf_maps = vec_expand(framebuf_maps, framebuf_maps_size);
		framebuf_maps[framebuf_maps_size++] = map;
	}

	*attach = GL_COLOR_ATTACHMENT0 + i;
	return 0;
}

/**
 * Prepare the color buffers attached to this state.
 **/
static void
state_prep_colorbufs(state_t *state)
{
	GLenum *buffers;
	size_t i;

	if (! state->num_colorbufs) {
		state_prep_default_colorbufs();
		return;
	}

	buffers = xcalloc(state->num_colorbufs, sizeof(GLenum));

	if (! framebuf_maps)
		glGenFramebuffers(1, &framebuf);

	if (current_state == NULL || current_state->num_colorbufs == 0)
		glBindFramebuffer(GL_FRAMEBUFFER, framebuf);

	i = 0;
	while (i < state->num_colorbufs) {
		if (! state_alloc_framebuffer(state->colorbufs[i],
					      &buffers[i])) {
			i++;
		} else {
			framebuf_maps_size = 0;
			i = 0;
		}
	}

	glDrawBuffers(state->num_colorbufs, buffers);
	free(buffers);
}

/**
 * Enter the given state.
 **/
void
state_enter(state_t *state)
{
	uint64_t change_flags = state->care_about;
	state_t *tmp = NULL;
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
	
	if (change_flags & STATE_TEXTURE_2D)
		state_set_texture_2D(state);

	state_prep_colorbufs(state);

	if (current_state)
		tmp = current_state;

	current_state = state;

	if (tmp)
		state_ungrab(tmp);
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

/**
 * Indicate that in the given state, the given flags may hold any value.
 **/
void
state_ignore_flags(state_t *state, uint64_t flags)
{
	state_assert_not_current(state);

	state->care_about &= ~flags;
}

/**
 * Append a color buffer to the list of output color buffers.
 **/
size_t
state_append_colorbuf(state_t *state, texmap_t *texture)
{
	state_assert_not_current(state);

	if (state->num_colorbufs == state_max_colorbufs())
		errx(1, "Platform does not support more than "
		     "%zu color buffers", state_max_colorbufs());

	vec_expand(state->colorbufs, state->num_colorbufs);
	state->colorbufs[state->num_colorbufs] = texture;
	texmap_grab(texture);

	return state->num_colorbufs++;
}

/**
 * Get the maximum number of color buffers a state can have.
 **/
size_t
state_max_colorbufs(void)
{
	GLint result;

	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &result);

	return (size_t)result;
}

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

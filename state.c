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

/**
 * Create a new state object.
 **/
state_t *
state_create(void)
{
	state_t *state = xmalloc(sizeof(state_t));
	memset(state, 0, sizeof(state_t));

	return state;
}

/**
 * Destroy a state object. Don't defer until the state is no longer active.
 **/
static void
state_do_destroy(state_t *state)
{
	free(state);
}

/**
 * Destroy a state object.
 **/
void
state_destroy(state_t *state)
{
	state->destroyed = 1;

	if (state != current_state)
		state_do_destroy(state);
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
state_set_texture_2d(state_t *state)
{
	if (state->flags & STATE_TEXTURE_2D)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);
}

/**
 * Enter the given state.
 **/
void
state_enter(state_t *state)
{
	uint64_t change_flags = state->care_about;
	
	if (current_state) {
		change_flags = current_state->flags;
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
		state_set_texture_2d(state);

	if (current_state && current_state->destroyed)
		state_do_destroy(current_state);

	current_state = state;
}

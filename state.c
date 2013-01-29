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
 * Enter the given state.
 **/
void
state_enter(state_t *state)
{
	int gl_clear_flags = 0;

	if (gl_clear_flags)
		glClear(gl_clear_flags);

	if (state->flags & STATE_DEPTH_TEST)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	if (state->flags & STATE_ALPHA_BLEND) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}
	
	if (state->flags & STATE_BF_CULL) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	} else {
		glDisable(GL_CULL_FACE);
	}

	if (state->flags & STATE_TEXTURE_2D)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);
}

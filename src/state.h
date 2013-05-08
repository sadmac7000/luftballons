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

#ifndef STATE_H
#define STATE_H
#include <luftballons/state.h>

#include "texmap.h"
#include "shader.h"
#include "refcount.h"
#include "colorbuf.h"

/* Enable depth testing in this state */
#define STATE_DEPTH_TEST	LUFT_STATE_DEPTH_TEST
/* Enable alpha blending in this state */
#define STATE_ALPHA_BLEND	LUFT_STATE_ALPHA_BLEND
/* Enable backface culling in this state */
#define STATE_BF_CULL		LUFT_STATE_BF_CULL
/* Enable the 2D texture buffer */
#define STATE_TEXTURE_2D	LUFT_STATE_TEXTURE_2D

/**
 * A bin of current misc. OpenGL state.
 *
 * flags: Flags indicating properties of this draw state.
 * care_about: Which flags we actually care about the state of.
 * colorbuf: Color buffer to render to.
 * num_uniforms, uniforms: Name-sorted vector of uniforms to install.
 * shader: Shader to load in this state.
 * mat_id: The material we draw.
 * refcount: Reference counter.
 **/
typedef struct state {
	uint64_t flags;
	uint64_t care_about;
	colorbuf_t *colorbuf;
	size_t num_uniforms;
	uniform_t **uniforms;
	shader_t *shader;
	int mat_id;
	refcounter_t refcount;
} state_t;

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(state_create);
API_DECLARE(state_grab);
API_DECLARE(state_ungrab);
API_DECLARE(state_set_flags);
API_DECLARE(state_clear_flags);
API_DECLARE(state_ignore_flags);
API_DECLARE(state_set_colorbuf);
API_DECLARE(state_set_uniform);
API_DECLARE(state_set_material);
API_DECLARE(state_max_colorbufs);

int state_material_active(int mat_id);
void state_enter(state_t *state);

#ifdef __cplusplus
}
#endif

#endif /* STATE_H */

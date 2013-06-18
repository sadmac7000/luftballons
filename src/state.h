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
#include "object.h"

/* Enable depth testing in this state */
#define STATE_DEPTH_TEST	LUFT_STATE_DEPTH_TEST
/* Enable backface culling in this state */
#define STATE_BF_CULL		LUFT_STATE_BF_CULL

typedef luft_state_blend_mode_t state_blend_mode_t;
#define STATE_BLEND_DONTCARE		LUFT_STATE_BLEND_DONTCARE
#define STATE_BLEND_NONE		LUFT_STATE_BLEND_NONE
#define STATE_BLEND_ALPHA		LUFT_STATE_BLEND_ALPHA
#define STATE_BLEND_REVERSE_ALPHA	LUFT_STATE_BLEND_REVERSE_ALPHA
#define STATE_BLEND_ADDITIVE		LUFT_STATE_BLEND_ADDITIVE

/**
 * A bin of current misc. OpenGL state.
 *
 * flags: Flags indicating properties of this draw state.
 * care_about: Which flags we actually care about the state of.
 * colorbuf: Color buffer to render to.
 * num_uniforms, uniforms: Vector of uniforms to install.
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
	state_blend_mode_t blend_mode;
	refcounter_t refcount;
} state_t;

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(state_create);
API_DECLARE(state_set_shader);
API_DECLARE(state_clone);
API_DECLARE(state_grab);
API_DECLARE(state_ungrab);
API_DECLARE(state_set_flags);
API_DECLARE(state_clear_flags);
API_DECLARE(state_ignore_flags);
API_DECLARE(state_set_colorbuf);
API_DECLARE(state_set_uniform);
API_DECLARE(state_set_material);
API_DECLARE(state_max_colorbufs);
API_DECLARE(state_set_blend);

int state_material_active(int mat_id);
void state_push(state_t *state);
void state_pop(state_t *state);

#ifdef __cplusplus
}
#endif

#endif /* STATE_H */

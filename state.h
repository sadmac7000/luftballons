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

#ifndef STATE_H
#define STATE_H

#include "texmap.h"
#include "shader.h"

/* Enable depth testing in this state */
#define STATE_DEPTH_TEST	0x1
/* Enable alpha blending in this state */
#define STATE_ALPHA_BLEND	0x2
/* Enable backface culling in this state */
#define STATE_BF_CULL		0x4
/* Enable the 2D texture buffer */
#define STATE_TEXTURE_2D	0x8

/**
 * A bin of current misc. OpenGL state.
 *
 * flags: Flags indicating properties of this draw state.
 * care_about: Which flags we actually care about the state of.
 * destroyed: Indicates this state has been destroyed.
 * num_colorbufs, colorbufs: Vector of color buffers to render to.
 * num_uniforms, uniforms: Name-sorted vector of uniforms to install.
 * num_dependants, dependants: Number of states depending on this state.
 * shader: Shader to load in this state.
 * mat_id: The material we draw.
 **/
typedef struct state {
	uint64_t flags;
	uint64_t care_about;
	int destroyed;
	size_t num_colorbufs;
	texmap_t **colorbufs;
	size_t num_uniforms;
	shader_uniform_t **uniforms;
	size_t num_dependants;
	size_t dependencies;
	shader_t *shader;
	int mat_id;
} state_t;

#ifdef __cplusplus
extern "C" {
#endif

state_t *state_create(shader_t *shader);
void state_enter(state_t *state);
void state_destroy(state_t *state);
void state_set_flags(state_t *state, uint64_t flags);
void state_clear_flags(state_t *state, uint64_t flags);
void state_ignore_flags(state_t *state, uint64_t flags);
size_t state_append_colorbuf(state_t *state, texmap_t *texture);
void state_clear_colorbufs(state_t *state);
void state_set_uniform(state_t *state, shader_uniform_t *uniform);
size_t state_max_colorbufs(void);
int state_material_active(int mat_id);
void state_depends_on(state_t *state, state_t *other);

#ifdef __cplusplus
}
#endif

#endif /* STATE_H */

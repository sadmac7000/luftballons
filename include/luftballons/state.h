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

#ifndef LUFTBALLONS_STATE_H
#define LUFTBALLONS_STATE_H

#include <luftballons/texmap.h>
#include <luftballons/shader.h>
#include <luftballons/colorbuf.h>

/* Enable depth testing in this state */
#define LUFT_STATE_DEPTH_TEST	0x1
/* Enable alpha blending in this state */
#define LUFT_STATE_ALPHA_BLEND	0x2
/* Enable backface culling in this state */
#define LUFT_STATE_BF_CULL	0x4

typedef struct state luft_state_t;

#ifdef __cplusplus
extern "C" {
#endif

luft_state_t *luft_state_create(luft_shader_t *shader);
void luft_state_grab(luft_state_t *state);
void luft_state_ungrab(luft_state_t *state);
void luft_state_set_flags(luft_state_t *state, uint64_t flags);
void luft_state_clear_flags(luft_state_t *state, uint64_t flags);
void luft_state_ignore_flags(luft_state_t *state, uint64_t flags);
void luft_state_set_colorbuf(luft_state_t *state, luft_colorbuf_t *colorbuf);
void luft_state_set_uniform(luft_state_t *state, luft_uniform_t *uniform);
void luft_state_set_material(luft_state_t *state, int mat_id);
size_t luft_state_max_colorbufs(void);

#ifdef __cplusplus
}
#endif

#endif /* LUFTBALLONS_STATE_H */

/**
 * Copyright © 2013 Casey Dahlin
 *
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
#include <luftballons/draw_op.h>

#include "texmap.h"
#include "shader.h"
#include "refcount.h"
#include "colorbuf.h"
#include "object.h"
#include "material.h"

/* Enable depth testing in this state */
#define STATE_DEPTH_TEST	LUFT_DEPTH_TEST
/* Enable backface culling in this state */
#define STATE_BF_CULL		LUFT_BF_CULL

typedef luft_blend_mode_t state_blend_mode_t;
#define STATE_BLEND_DONTCARE		LUFT_BLEND_DONTCARE
#define STATE_BLEND_NONE		LUFT_BLEND_NONE
#define STATE_BLEND_ALPHA		LUFT_BLEND_ALPHA
#define STATE_BLEND_REVERSE_ALPHA	LUFT_BLEND_REVERSE_ALPHA
#define STATE_BLEND_ADDITIVE		LUFT_BLEND_ADDITIVE

struct material;

/**
 * A bin of current misc. OpenGL state.
 *
 * flags: Flags indicating properties of this draw state.
 * care_about: Which flags we actually care about the state of.
 * colorbuf: Color buffer to render to.
 * shader: Shader to load in this state.
 * material_gen: Material backlog generation.
 * material, num_materials: The materials we draw.
 * refcount: Reference counter.
 **/
typedef struct state {
	uint64_t flags;
	uint64_t care_about;
	colorbuf_t *colorbuf;
	shader_t *shader;
	size_t material_gen;
	struct material *materials;
	size_t num_materials;
	state_blend_mode_t blend_mode;
	refcounter_t refcount;
} state_t;

#ifdef __cplusplus
extern "C" {
#endif

int state_material_active(material_t mat);
void state_print_current(void);
void state_material_activate(material_t mat);
void state_material_eliminate(state_t *state, material_t mat);
void state_push(state_t *state, material_t mat);
void state_pop(state_t *state, material_t mat);
state_t *state_create(void);
void state_set_shader(state_t *state, shader_t *shader);
state_t *state_clone(state_t *in);
void state_set_blend(state_t *state, state_blend_mode_t mode);
void state_grab(state_t *state);
void state_ungrab(state_t *state);
void state_set_flags(state_t *state, uint64_t flags);
void state_clear_flags(state_t *state, uint64_t flags);
void state_ignore_flags(state_t *state, uint64_t flags);
void state_set_colorbuf(state_t *state, colorbuf_t *colorbuf);
void state_set_uniform(state_t *state, material_t mat, uniform_type_t type,
			    ...);
size_t state_max_colorbufs(void);

#ifdef __cplusplus
}
#endif

#endif /* STATE_H */

/**
 * Copyright © 2013 Casey Dahlin
 *
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Luftballons is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Luftballons.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef LUFTBALLONS_DRAW_OP_H
#define LUFTBALLONS_DRAW_OP_H

#include <luftballons/object.h>
#include <luftballons/shader.h>
#include <luftballons/uniform.h>
#include <luftballons/colorbuf.h>
#include <luftballons/material.h>

/* Enable depth testing in this state */
#define LUFT_DEPTH_TEST		0x1
/* Enable backface culling in this state */
#define LUFT_BF_CULL		0x2

typedef enum {
	LUFT_BLEND_DONTCARE,
	LUFT_BLEND_NONE,
	LUFT_BLEND_ALPHA,
	LUFT_BLEND_REVERSE_ALPHA,
	LUFT_BLEND_ADDITIVE,
} luft_blend_mode_t;

typedef struct draw_op luft_draw_op_t;

#ifdef __cplusplus
extern "C" {
#endif

luft_draw_op_t *luft_draw_op_create(luft_object_t *object,
				    luft_object_t *camera);
luft_draw_op_t *luft_draw_op_clone(luft_draw_op_t *op);
void luft_draw_op_set_shader(luft_draw_op_t *op, luft_shader_t *shader);
void luft_draw_op_set_blend(luft_draw_op_t *op, luft_blend_mode_t mode);
void luft_draw_op_set_flags(luft_draw_op_t *op, uint64_t flags);
void luft_draw_op_clear_flags(luft_draw_op_t *op, uint64_t flags);
void luft_draw_op_ignore_flags(luft_draw_op_t *op, uint64_t flags);
void luft_draw_op_set_colorbuf(luft_draw_op_t *op, luft_colorbuf_t *colorbuf);
void luft_draw_op_activate_material(luft_draw_op_t *draw_op,
				    luft_material_t mat);
void luft_draw_op_deactivate_material(luft_draw_op_t *draw_op,
				      luft_material_t mat);
void luft_draw_op_set_uniform(luft_draw_op_t *op, luft_material_t mat,
			      luft_uniform_type_t type, ...);
void luft_draw_op_grab(luft_draw_op_t *op);
void luft_draw_op_ungrab(luft_draw_op_t *op);
void luft_draw_op_exec(luft_draw_op_t *op);

#ifdef __cplusplus
}
#endif
#endif /* LUFTBALLONS_DRAW_OP_H */

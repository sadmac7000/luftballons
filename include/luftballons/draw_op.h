/**
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
#include <luftballons/state.h>

typedef struct draw_op luft_draw_op_t;

#ifdef __cplusplus
extern "C" {
#endif

luft_draw_op_t *luft_draw_op_create(luft_object_t *object,
				    luft_object_t *camera,
				    luft_state_t *state);
void luft_draw_op_set_shader(luft_draw_op_t *op, luft_shader_t *shader);
void luft_draw_op_set_blend(luft_draw_op_t *op, luft_blend_mode_t mode);
void luft_draw_op_set_flags(luft_draw_op_t *op, uint64_t flags);
void luft_draw_op_clear_flags(luft_draw_op_t *op, uint64_t flags);
void luft_draw_op_ignore_flags(luft_draw_op_t *op, uint64_t flags);
void luft_draw_op_set_colorbuf(luft_draw_op_t *op, luft_colorbuf_t *colorbuf);
void luft_draw_op_set_uniform(luft_draw_op_t *op, luft_uniform_type_t type,
			      ...);
void luft_draw_op_set_material(luft_draw_op_t *op, int mat_id);
void luft_draw_op_grab(luft_draw_op_t *op);
void luft_draw_op_ungrab(luft_draw_op_t *op);
void luft_draw_op_exec(luft_draw_op_t *op);

#ifdef __cplusplus
}
#endif
#endif /* LUFTBALLONS_DRAW_OP_H */

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

#ifndef LUFTBALLONS_TARGET_H
#define LUFTBALLONS_TARGET_H

#include <luftballons/state.h>
#include <luftballons/object.h>
#include <luftballons/colorbuf.h>
#include <luftballons/draw_op.h>
#include <luftballons/shader.h>

typedef struct target luft_target_t;

#ifdef __cplusplus
extern "C" {
#endif

luft_target_t *luft_target_create(size_t repeat);
void luft_target_grab(luft_target_t *target);
void luft_target_ungrab(luft_target_t *target);
void luft_target_clear(luft_target_t *target, luft_colorbuf_t *buf);
void luft_target_draw(luft_target_t *target, luft_draw_op_t *op);
void luft_target_hit(luft_target_t *target);
void luft_target_hit_other(luft_target_t *target, luft_target_t *other);
void luft_target_set_shader(luft_target_t *target, luft_shader_t *shader);
void luft_target_set_blend(luft_target_t *target, luft_blend_mode_t mode);
void luft_target_set_flags(luft_target_t *target, uint64_t flags);
void luft_target_clear_flags(luft_target_t *target, uint64_t flags);
void luft_target_ignore_flags(luft_target_t *target, uint64_t flags);
void luft_target_set_colorbuf(luft_target_t *target, luft_colorbuf_t *colorbuf);
void luft_target_set_uniform(luft_target_t *target, luft_uniform_type_t type,
			      ...);
void luft_target_set_material(luft_target_t *target, int mat_id);
#ifdef __cplusplus
}
#endif

#endif /* LUFTBALLONS_TARGET_H */

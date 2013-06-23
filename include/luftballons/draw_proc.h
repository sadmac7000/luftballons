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

#ifndef LUFTBALLONS_DRAW_PROC_H
#define LUFTBALLONS_DRAW_PROC_H

#include <luftballons/object.h>
#include <luftballons/colorbuf.h>
#include <luftballons/draw_op.h>
#include <luftballons/shader.h>

typedef struct draw_proc luft_draw_proc_t;

#ifdef __cplusplus
extern "C" {
#endif

luft_draw_proc_t *luft_draw_proc_create(size_t repeat);
void luft_draw_proc_grab(luft_draw_proc_t *draw_proc);
void luft_draw_proc_ungrab(luft_draw_proc_t *draw_proc);
void luft_draw_proc_clear(luft_draw_proc_t *draw_proc, luft_colorbuf_t *buf);
void luft_draw_proc_draw(luft_draw_proc_t *draw_proc, luft_draw_op_t *op);
void luft_draw_proc_hit(luft_draw_proc_t *draw_proc);
void luft_draw_proc_hit_other(luft_draw_proc_t *draw_proc,
			      luft_draw_proc_t *other);
void luft_draw_proc_set_shader(luft_draw_proc_t *draw_proc,
			       luft_shader_t *shader);
void luft_draw_proc_set_blend(luft_draw_proc_t *draw_proc,
			      luft_blend_mode_t mode);
void luft_draw_proc_set_flags(luft_draw_proc_t *draw_proc, uint64_t flags);
void luft_draw_proc_clear_flags(luft_draw_proc_t *draw_proc, uint64_t flags);
void luft_draw_proc_ignore_flags(luft_draw_proc_t *draw_proc, uint64_t flags);
void luft_draw_proc_set_colorbuf(luft_draw_proc_t *draw_proc,
				 luft_colorbuf_t *colorbuf);
void luft_draw_proc_set_uniform(luft_draw_proc_t *draw_proc,
				luft_uniform_type_t type, ...);
void luft_draw_proc_set_material(luft_draw_proc_t *draw_proc, int mat_id);
#ifdef __cplusplus
}
#endif

#endif /* LUFTBALLONS_DRAW_PROC_H */

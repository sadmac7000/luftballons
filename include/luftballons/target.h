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

typedef struct target luft_target_t;

#ifdef __cplusplus
extern "C" {
#endif

luft_target_t *luft_target_create(luft_object_t *camera, luft_state_t *base,
				  size_t repeat);
void luft_target_grab(luft_target_t *target);
void luft_target_ungrab(luft_target_t *target);
void luft_target_add_dep(luft_target_t *target, luft_target_t *dep);
void luft_target_add_seq_dep(luft_target_t *target, luft_target_t *dep);
void luft_target_add_state(luft_target_t *target, luft_state_t *state);
void luft_target_clear_buf(luft_target_t *target, luft_colorbuf_t *buf);
void luft_target_draw(luft_target_t *target, luft_draw_op_t *op);
void luft_target_hit(luft_target_t *target);

#ifdef __cplusplus
}
#endif

#endif /* LUFTBALLONS_TARGET_H */

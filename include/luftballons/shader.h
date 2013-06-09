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

#ifndef LUFTBALLONS_SHADER_H
#define LUFTBALLONS_SHADER_H

#include <luftballons/uniform.h>

typedef struct shader luft_shader_t;

#ifdef __cplusplus
extern "C" {
#endif

luft_shader_t *luft_shader_create(const char *vertex, const char *frag);
void luft_shader_grab(luft_shader_t *shader);
void luft_shader_ungrab(luft_shader_t *shader);
void luft_shader_activate(luft_shader_t *shader);
void luft_shader_set_uniform(luft_shader_t *shader, luft_uniform_t *uniform);
void luft_shader_set_temp_uniform(luft_uniform_t *uniform);

#ifdef __cplusplus
}
#endif

#endif /* LUFTBALLONS_SHADER_H */

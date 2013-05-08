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

#ifndef LUFTBALLONS_TEXMAP_H
#define LUFTBALLONS_TEXMAP_H

#include <GL/gl.h>

typedef struct texmap luft_texmap_t;

#ifdef __cplusplus
extern "C" {
#endif

luft_texmap_t *luft_texmap_create(size_t base_level, size_t max_level,
				  int compress);
void luft_texmap_load_image(luft_texmap_t *map, const char *path, int level);
void luft_texmap_init_blank(luft_texmap_t *map, int level, int width,
			    int height);
void luft_texmap_set_int_param(luft_texmap_t *map, GLenum param, GLint value);
void luft_texmap_grab(luft_texmap_t *luft_texmap);
void luft_texmap_ungrab(luft_texmap_t *luft_texmap);

#ifdef __cplusplus
}
#endif

#endif /* LUFTBALLONS_TEXMAP_H */


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

#include <sys/types.h>

#define LUFT_TEXMAP_COMPRESSED	0x1
#define LUFT_TEXMAP_FLOAT32	0x2
#define LUFT_TEXMAP_DEPTH	0x4
#define LUFT_TEXMAP_STENCIL	0x8

#define LUFT_TEXMAP_WRAP_R 0x1
#define LUFT_TEXMAP_WRAP_S 0x2
#define LUFT_TEXMAP_WRAP_T 0x4

#define LUFT_TEXMAP_WRAP_CLAMP  0x1
#define LUFT_TEXMAP_WRAP_MIRROR 0x2
#define LUFT_TEXMAP_WRAP_REPEAT 0x3

typedef struct texmap luft_texmap_t;
typedef enum {
	LUFT_TEXMAP_INTERP_NONE,
	LUFT_TEXMAP_INTERP_NEAREST,
	LUFT_TEXMAP_INTERP_LINEAR,
} luft_texmap_interp_t;

#ifdef __cplusplus
extern "C" {
#endif

luft_texmap_t *luft_texmap_create(size_t base_level, size_t max_level,
				  unsigned int flags);
void luft_texmap_load_image(luft_texmap_t *map, const char *path, int level);
void luft_texmap_init_blank(luft_texmap_t *map, int level, int width,
			    int height);
void luft_texmap_set_mag(luft_texmap_t *map, luft_texmap_interp_t interp);
void luft_texmap_set_min(luft_texmap_t *map, luft_texmap_interp_t local,
			 luft_texmap_interp_t mip);
void luft_texmap_set_wrap(luft_texmap_t *map, unsigned int wrap_axes,
			  unsigned int wrap_type);
void luft_texmap_grab(luft_texmap_t *luft_texmap);
void luft_texmap_ungrab(luft_texmap_t *luft_texmap);

#ifdef __cplusplus
}
#endif

#endif /* LUFTBALLONS_TEXMAP_H */


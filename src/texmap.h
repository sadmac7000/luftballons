/**
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Luftballons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Luftballons.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef TEXMAP_H
#define TEXMAP_H

#include <GL/gl.h>

#include "refcount.h"

#define TEXMAP_COMPRESSED	0x1
#define TEXMAP_INITIALIZED	0x2

/**
 * A 2D texture map, usually loaded from a file.
 *
 * map: OpenGL texture ID
 * sampler: OpenGL sampler object
 * flags: Misc flags.
 * w, h: Width and height
 * texture_unit: What texture unit this texture was assigned.
 * refcount: Number of references to this texture
 **/
typedef struct texmap {
	GLuint map;
	GLuint sampler;
	unsigned int flags;
	size_t w, h;
	GLuint texture_unit;
	refcounter_t refcount;
} texmap_t;

#ifdef __cplusplus
extern "C" {
#endif

texmap_t *texmap_create(size_t base_level, size_t max_level, int compress);
void texmap_load_image(texmap_t *map, const char *path, int level);
void texmap_init_blank(texmap_t *map, int level, int width, int height);
void texmap_set_int_param(texmap_t *map, GLenum param, GLint value);
void texmap_grab(texmap_t *texmap);
void texmap_ungrab(texmap_t *texmap);
size_t texmap_get_texture_unit(texmap_t *texmap);
void texmap_end_unit_generation(void);

#ifdef __cplusplus
}
#endif

#endif /* TEXMAP_H */

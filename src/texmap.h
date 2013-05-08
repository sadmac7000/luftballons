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

#ifndef TEXMAP_H
#define TEXMAP_H
#include <luftballons/texmap.h>

#include <GL/gl.h>

#include "refcount.h"
#include "util.h"

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

API_DECLARE(texmap_create);
API_DECLARE(texmap_load_image);
API_DECLARE(texmap_init_blank);
API_DECLARE(texmap_set_int_param);
API_DECLARE(texmap_grab);
API_DECLARE(texmap_ungrab);

size_t texmap_get_texture_unit(texmap_t *texmap);
void texmap_end_unit_generation(void);

#ifdef __cplusplus
}
#endif

#endif /* TEXMAP_H */

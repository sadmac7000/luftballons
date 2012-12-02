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

/**
 * A 2D texture map, usually loaded from a file.
 *
 * map: OpenGL texture ID
 * sampler: OpenGL sampler object
 **/
typedef struct texmap {
	GLuint map;
	GLuint sampler;
} texmap_t;

#ifdef __cplusplus
extern "C" {
#endif

texmap_t *texmap_create(size_t base_level, size_t max_level);
void texmap_load_image(texmap_t *map, const char *path, int level);
void texmap_set_int_param(texmap_t *map, GLenum param, GLint value);

#ifdef __cplusplus
}
#endif

#endif /* TEXMAP_H */

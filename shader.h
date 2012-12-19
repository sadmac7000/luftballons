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

#ifndef SHADER_H
#define SHADER_H

#include <GL/gl.h>

#include "vbuf.h"
#include "texmap.h"

/**
 * A shader.
 *
 * gl_handle: The OpenGL designation for the shader.
 * tex_unit: The next available texture unit.
 **/
typedef struct shader {
	GLuint gl_handle;
	size_t tex_unit;
} shader_t;

typedef enum {
	SHADER_UNIFORM_MAT4,
	SHADER_UNIFORM_VEC4,
	SHADER_UNIFORM_SAMP2D,
	SHADER_UNIFORM_SAMP1D,
	SHADER_UNIFORM_UINT,
} shader_uniform_type_t;

#ifdef __cplusplus
extern "C" {
#endif

shader_t *shader_create(const char *vertex, const char *frag);
void shader_activate(shader_t *shader);
void shader_notify_draw(void);
void shader_set_uniform_mat(shader_t *shader, const char *name, float mat[16]);
void shader_set_uniform_samp2D(shader_t *shader, const char *name, texmap_t *map);
void shader_set_uniform_uint(shader_t *shader, const char *name, GLuint val);
void shader_set_uniform_vec(shader_t *shader, const char *name, float vec[4]);

#ifdef __cplusplus
}
#endif

#endif /* SHADER_H */

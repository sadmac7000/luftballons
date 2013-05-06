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
#include "refcount.h"
#include "uniform.h"

/**
 * A shader.
 *
 * gl_handle: The OpenGL designation for the shader.
 * uniforms, uniform_count: Vector of uniforms applied to this shader.
 **/
typedef struct shader {
	GLuint gl_handle;
	uniform_t **uniforms;
	size_t uniform_count;
} shader_t;

#ifdef __cplusplus
extern "C" {
#endif

shader_t *shader_create(const char *vertex, const char *frag);
void shader_activate(shader_t *shader);
void shader_set_uniform(shader_t *shader, uniform_t *uniform);
void shader_set_temp_uniform(uniform_t *uniform);

#ifdef __cplusplus
}
#endif

#endif /* SHADER_H */

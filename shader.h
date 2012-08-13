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

#include <GL/glut.h>

/**
 * Callback that recieves a vertex attribute name and sets its buffer location.
 *
 * name: The name of the attribute.
 * id: The opengl handle of the attribute.
 *
 * Returns: Nonzero if the attribute is used.
 **/
typedef int (*shader_attr_callback_t)(const char *name, GLuint id, void *data);

/**
 * A shader.
 *
 * gl_handle: The OpenGL designation for the shader.
 **/
typedef struct shader {
	GLuint gl_handle;
} shader_t;

#ifdef __cplusplus
extern "C" {
#endif

shader_t *shader_create(const char *vertex, const char *frag);
void shader_activate(shader_t *shader);
void shader_set_vertex_attrs(shader_t *shader,
			     shader_attr_callback_t callback, void *data);

#ifdef __cplusplus
}
#endif

#endif /* SHADER_H */

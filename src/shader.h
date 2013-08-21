/**
 * Copyright © 2013 Casey Dahlin
 *
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

#ifndef SHADER_H
#define SHADER_H
#include <luftballons/shader.h>

#include <GL/gl.h>

#include "vbuf.h"
#include "texmap.h"
#include "refcount.h"
#include "uniform.h"
#include "util.h"

/**
 * A shader.
 *
 * gl_handle: The OpenGL designation for the shader.
 * uniforms, uniform_count: Vector of uniforms applied to this shader.
 * refcount: Reference counter for this state.
 **/
typedef struct shader {
	GLuint gl_handle;
	uniform_t **uniforms;
	size_t uniform_count;

	refcounter_t refcount;
} shader_t;

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(shader_create);
API_DECLARE(shader_grab);
API_DECLARE(shader_ungrab);

void shader_activate(shader_t *shader);
void shader_set_uniform(shader_t *shader, uniform_t *uniform);
void shader_set_temp_uniform(uniform_t *uniform);

#ifdef __cplusplus
}
#endif

#endif /* SHADER_H */

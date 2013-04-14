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

#ifndef UNIFORM_H
#define UNIFORM_H

#include <GL/gl.h>

#include "refcount.h"

/**
 * Type of a shader uniform.
 **/
typedef enum {
	UNIFORM_MAT4,
	UNIFORM_VEC4,
	UNIFORM_SAMP2D,
	UNIFORM_SAMP1D,
	UNIFORM_UINT,
} uniform_type_t;

/**
 * Uniform values.
 **/
typedef union uniform_value {
	void *data_ptr;
	GLuint uint;
} uniform_value_t;

/**
 * A uniform.
 **/
typedef struct uniform {
	char *name;
	uniform_type_t type;
	uniform_value_t value;
	refcounter_t refcount;
} uniform_t;

#ifdef __cplusplus
extern "C" {
#endif

uniform_t *uniform_create(const char *name,
			  uniform_type_t type,
			  uniform_value_t value);
void uniform_grab(uniform_t *uniform);
void uniform_ungrab(uniform_t *uniform);

#ifdef __cplusplus
}
#endif

#endif /* UNIFORM_H */

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

#ifndef UNIFORM_H
#define UNIFORM_H
#include <luftballons/uniform.h>

#include <stdarg.h>

#include <GL/gl.h>

#include "refcount.h"
#include "util.h"

#define UNIFORM_CLONE LUFT_UNIFORM_CLONE
#define UNIFORM_MAT4 LUFT_UNIFORM_MAT4
#define UNIFORM_VEC4 LUFT_UNIFORM_VEC4
#define UNIFORM_TEXMAP LUFT_UNIFORM_TEXMAP
#define UNIFORM_UINT LUFT_UNIFORM_UINT

typedef luft_uniform_type_t uniform_type_t;

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

API_DECLARE(uniform_create);
API_DECLARE(uniform_grab);
API_DECLARE(uniform_ungrab);

uniform_t *uniform_vcreate(uniform_type_t type, va_list ap);

#ifdef __cplusplus
}
#endif

#endif /* UNIFORM_H */

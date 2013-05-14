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

#ifndef LUFTBALLONS_UNIFORM_H
#define LUFTBALLONS_UNIFORM_H

#include <GL/gl.h>

typedef struct uniform luft_uniform_t;

/**
 * Type of a shader uniform.
 **/
typedef enum {
	LUFT_UNIFORM_MAT4,
	LUFT_UNIFORM_VEC4,
	LUFT_UNIFORM_SAMP2D,
	LUFT_UNIFORM_SAMP1D,
	LUFT_UNIFORM_UINT,
} luft_uniform_type_t;

/**
 * Uniform values.
 **/
typedef union uniform_value {
	void *data_ptr;
	GLuint uint;
} luft_uniform_value_t;

#ifdef __cplusplus
extern "C" {
#endif

luft_uniform_t *luft_uniform_create(const char *name,
				    luft_uniform_type_t type,
				    luft_uniform_value_t value);
void luft_uniform_grab(luft_uniform_t *uniform);
void luft_uniform_ungrab(luft_uniform_t *uniform);

#ifdef __cplusplus
}
#endif

#endif /* LUFTBALLONS_UNIFORM_H */


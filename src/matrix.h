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

#ifndef MATRIX_H
#define MATRIX_H
#include <luftballons/matrix.h>

#include "util.h"

#define MATRIX_DECL LUFT_MATRIX_DECL
#define MATRIX_DECL_IDENT LUFT_MATRIX_DECL_IDENT

/**
 * A stack of matrices.
 *
 * data: A buffer for the frames.
 * size: Number of frames on the stack.
 **/
typedef struct matrix_stack {
	float *data;
	size_t size;
} matrix_stack_t;

#define MATRIX_STACK_DECL LUFT_MATRIX_STACK_DECL

#define matrix_print luft_matrix_print
#define matrix_dup luft_matrix_dup
#define matrix_ident luft_matrix_ident

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(matrix_stack_release);
API_DECLARE(matrix_transpose);
API_DECLARE(matrix_inverse_trans);
API_DECLARE(matrix_multiply);
API_DECLARE(matrix_vec3_mul);
API_DECLARE(vec3_dot);
API_DECLARE(vec3_cross);
API_DECLARE(vec3_normalize);
API_DECLARE(vec3_magnitude);
API_DECLARE(vec3_add);
API_DECLARE(vec3_subtract);
API_DECLARE(vec3_scale);
API_DECLARE(vec3_dup);
API_DECLARE(matrix_stack_push);
API_DECLARE(matrix_stack_pop);

#ifdef __cplusplus
}
#endif

#endif /* MATRIX_H */




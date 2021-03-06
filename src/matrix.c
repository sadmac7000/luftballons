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

#include <math.h>
#include <string.h>

#include "matrix.h"
#include "util.h"

/**
 * Free a matrix stack's data.
 **/
void
matrix_stack_release(matrix_stack_t *stack)
{
	free(stack->data);
}
EXPORT(matrix_stack_release);

/**
 * Push an item onto the matrix stack.
 **/
void
matrix_stack_push(matrix_stack_t *stack, float item[16])
{
	float *target;

	stack->data = do_vec_expand(stack->data, stack->size,
				    16 * sizeof(float));

	target = stack->data + 16 * stack->size++;
	memcpy(target, item, 16 * sizeof(float));
}
EXPORT(matrix_stack_push);

/**
 * Pop an item from the matrix stack.
 *
 * stack: Stack to pop from
 * item: Where to store item to pop. May be NULL.
 *
 * Returns: 0 on success, -1 on empty stack.
 **/
int
matrix_stack_pop(matrix_stack_t *stack, float item[16])
{
	float *target;

	if (! stack->size)
		return -1;

	target = stack->data + 16 * --stack->size;

	if (item)
		memcpy(item, target, 16 * sizeof(float));

	stack->data = do_vec_contract(stack->data, stack->size,
				      16 * sizeof(float));

	return 0;
}
EXPORT(matrix_stack_pop);

/**
 * Find a diagonalized product for a 3x3 submatrix of a matrix.
 **/
static inline float
matrix_subdiag(float in[16], size_t i, size_t j, size_t col, int inc)
{
	size_t row = i ? 0:1;
	size_t l;
	float res = 1;

	if (col >= j)
		col++;
	for (l = 0; l < 3; l++, row++) {
		if (row == i)
			row++;

		if (col == j) {
			if (col == 0 && inc < 0)
				col = 4;
			col += inc;
			col %= 4;
		}

		res *= in[row + col * 4];

		if (col == 0 && inc < 0)
			col = 4;
		col += inc;
		col %= 4;
	}

	return res;
}

/**
 * Find the given minor for a matrix.
 **/
static inline float
matrix_minor(float in[16], size_t i, size_t j)
{
	return matrix_subdiag(in, i, j, 0, 1) +
		matrix_subdiag(in, i, j, 1, 1) +
		matrix_subdiag(in, i, j, 2, 1) -
		matrix_subdiag(in, i, j, 0, -1) -
		matrix_subdiag(in, i, j, 1, -1) -
		matrix_subdiag(in, i, j, 2, -1);
}

/**
 * Find the given cofactor for a matrix.
 **/
static inline float
matrix_cofactor(float in[16], size_t i, size_t j)
{
	if ((i + j) & 1)
		return -matrix_minor(in, i, j);
	else
		return matrix_minor(in, i, j);
}

/**
 * Find the determinant of a matrix.
 **/
float
matrix_determinant(float in[16])
{
	float det = 0;
	size_t i;

	for (i = 0; i < 4; i++)
		det += matrix_cofactor(in, i, 0) * in[i];

	return det;
}
EXPORT(matrix_determinant);

/**
 * Invert and transpose matrix.
 *
 * Returns: True if the matrix has an inverse.
 **/
int
matrix_inverse_trans(float in[16], float out[16])
{
	float *result = out;
	float tmp[16];
	size_t i, j;
	float det = 0;

	if (out == in)
		result = tmp;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			result[i + j * 4] = matrix_cofactor(in, i, j);

	for (i = 0; i < 4; i++)
		det += in[i] * result[i];

	/* FIXME: Small floating point error might bury us here. What to do? */
	if (det == 0)
		return 0;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			result[i + j * 4] /= det;

	if (result != out)
		memcpy(out, result, 16 * sizeof(float));

	return 1;
}
EXPORT(matrix_inverse_trans);

/**
 * Transpose a matrix.
 **/
void
matrix_transpose(float in[16], float out[16])
{
	float tmp_out[16];
	int i, j;

	if (in == out) {
		matrix_transpose(in, tmp_out);
		memcpy(out, tmp_out, 16 * sizeof(float));
		return;
	}

	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			out[i*4 + j] = in[j * 4 + i];
}
EXPORT(matrix_transpose);

/**
 * Multiply two matricies
 **/
void
matrix_multiply(float a[16], float b[16], float result[16])
{
	float tmp_out[16];
	int i, j, k;

	if (a == result || b == result) {
		matrix_multiply(a, b, tmp_out);
		memcpy(result, tmp_out, 16 * sizeof(float));
		return;
	}

	memset(result, 0, 16 * sizeof(float));

	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			for (k = 0; k < 4; k++)
				result[i + j * 4] += a[i + k * 4] * b[k + j * 4];
}
EXPORT(matrix_multiply);

/**
 * Multiply a matrix by a vector. The vector is expanded from a 3-vector to a
 * 4-vector with the given W component.
 **/
void
matrix_vec3_mul(float mat[16], float vec[3], float w, float out[4])
{
	float vec4[] = { vec[0], vec[1], vec[2], w };
	float tmp_out[4];
	int i, k;

	if (vec == out) {
		matrix_vec3_mul(mat, vec, w, tmp_out);
		memcpy(out, tmp_out, 4 * sizeof(float));
		return;
	}

	memset(out, 0, 4 * sizeof(float));

	for (i = 0; i < 4; i++)
		for (k = 0; k < 4; k++)
			out[i] += mat[i + k * 4] * vec4[k];
}
EXPORT(matrix_vec3_mul);

/**
 * Dot product of two vectors.
 **/
float
vec3_dot(float a[3], float b[3])
{
	return a[0] * b[0] + a[1]*b[1] + a[2] * b[2];
}
EXPORT(vec3_dot);

/**
 * Cross product of two vectors.
 **/
void
vec3_cross(float a[3], float b[3], float result[3])
{
	float tmp_res[3];
	float *res = result;

	if (a == result || b == result)
		res = tmp_res;

	res[0] = a[1] * b[2] - a[2] * b[1];
	res[1] = a[2] * b[0] - a[0] * b[2];
	res[2] = a[0] * b[1] - a[1] * b[0];

	if (a == result || b == result)
		vec3_dup(tmp_res, result);
}
EXPORT(vec3_cross);

/**
 * Magnitude of a vector.
 **/
float
vec3_magnitude(float in[3])
{
	return sqrtf(in[0] * in[0] + in[1] * in[1] + in[2] * in[2]);
}
EXPORT(vec3_magnitude);

/**
 * Normalize a vector.
 **/
void
vec3_normalize(float in[3], float out[3])
{
	float mag = vec3_magnitude(in);

	vec3_scale(in, out, 1.0 / mag);
}
EXPORT(vec3_normalize);

/**
 * Add a vector to a vector.
 **/
void
vec3_add(float a[3], float b[3], float result[3])
{
	result[0] = a[0] + b[0];
	result[1] = a[1] + b[1];
	result[2] = a[2] + b[2];
}
EXPORT(vec3_add);

/**
 * Duplicate a vector.
 **/
void
vec3_dup(float in[3], float out[3])
{
	memcpy(out, in, 3 * sizeof(float));
}
EXPORT(vec3_dup);

/**
 * Subtract a vector from a vector.
 **/
void
vec3_subtract(float a[3], float b[3], float result[3])
{
	result[0] = a[0] - b[0];
	result[1] = a[1] - b[1];
	result[2] = a[2] - b[2];
}
EXPORT(vec3_subtract);

/**
 * Scale a vector by a factor.
 **/
void
vec3_scale(float in[3], float out[3], float factor)
{
	out[0] = in[0] * factor;
	out[1] = in[1] * factor;
	out[2] = in[2] * factor;
}
EXPORT(vec3_scale);

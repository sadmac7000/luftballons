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

#include <math.h>
#include <string.h>

#include "matrix.h"
#include "util.h"

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

/**
 * Dot product of two vectors.
 **/
float
vec3_dot(float a[3], float b[3])
{
	return a[0] * b[0] + a[1]*b[1] + a[2] * b[2];
}

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

/**
 * Magnitude of a vector.
 **/
float
vec3_magnitude(float in[3])
{
	return sqrtf(in[0] * in[0] + in[1] * in[1] + in[2] * in[2]);
}

/**
 * Normalize a vector.
 **/
void
vec3_normalize(float in[3], float out[3])
{
	float mag = vec3_magnitude(in);

	vec3_scale(in, out, 1.0 / mag);
}

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

/**
 * Duplicate a vector.
 **/
void
vec3_dup(float in[3], float out[3])
{
	memcpy(out, in, 3 * sizeof(float));
}

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

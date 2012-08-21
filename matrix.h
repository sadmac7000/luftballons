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

#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <string.h>

#define MATRIX_DECL(matrix, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
	float matrix[16] = { a,e,i,m, b,f,j,n, c,g,k,o, d,h,l,p }

#define MATRIX_DECL_IDENT(matrix) MATRIX_DECL(matrix,  \
					      1,0,0,0, \
					      0,1,0,0, \
					      0,0,1,0, \
					      0,0,0,1)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Print out a matrix for debugging.
 **/
static inline void matrix_print(float m[16])
{
	printf("%f %f %f %f\n", m[0], m[4], m[8], m[12]);
	printf("%f %f %f %f\n", m[1], m[5], m[9], m[13]);
	printf("%f %f %f %f\n", m[2], m[6], m[10], m[14]);
	printf("%f %f %f %f\n", m[3], m[7], m[11], m[15]);
}

/**
 * Write an identity matrix.
 **/
static inline void matrix_ident(float out[16])
{
	memset(out, 0, 16 * sizeof(float));
	out[0] = out[5] = out[10] = out[15] = 1;
}

void matrix_transpose(float in[16], float out[16]);
void matrix_multiply(float a[16], float b[16], float result[16]);
float vec3_dot(float a[3], float b[3]);
void vec3_cross(float a[3], float b[3], float result[3]);
void vec3_normalize(float in[3], float out[3]);
float vec3_magnitude(float in[3]);
void vec3_add(float a[3], float b[3], float result[3]);
void vec3_subtract(float a[3], float b[3], float result[3]);
void vec3_scale(float in[3], float out[3], float factor);
void vec3_dup(float in[3], float out[3]);

#ifdef __cplusplus
}
#endif

#endif /* MATRIX_H */




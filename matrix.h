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

#ifdef __cplusplus
extern "C" {
#endif

void matrix_transpose(float in[16], float out[16]);
void matrix_multiply(float a[16], float b[16], float result[16]);
float vec3_dot(float a[3], float b[3]);
void vec3_cross(float a[3], float b[3], float result[3]);
void vec3_normalize(float in[3], float out[3]);
float vec3_magnitude(float in[3]);
void vec3_subtract(float a[3], float b[3], float result[3]);
void vec3_scale(float in[3], float out[3], float factor);

#ifdef __cplusplus
}
#endif

#endif /* MATRIX_H */




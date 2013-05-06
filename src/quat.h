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

#include "matrix.h"

#ifndef QUAT_H
#define QUAT_H

typedef struct quat {
	float c[4];
	size_t mul_count;
} quat_t;

#ifdef __cplusplus
extern "C" {
#endif

void quat_init(quat_t *quat, float x, float y, float z, float theta);
void quat_mul(quat_t *a, quat_t *b, quat_t *out);
void quat_dup(quat_t *in, quat_t *out);
void quat_to_matrix(quat_t *quat, float m[16]);

#ifdef __cplusplus
}
#endif

#endif /* QUAT_H */



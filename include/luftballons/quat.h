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

#ifndef LUFTBALLONS_QUAT_H
#define LUFTBALLONS_QUAT_H

#include <stdlib.h>

typedef struct quat {
	float c[4];
	size_t mul_count;
} luft_quat_t;

#ifdef __cplusplus
extern "C" {
#endif

void luft_quat_init(luft_quat_t *quat, float x, float y, float z, float theta);
void luft_quat_init_euler(luft_quat_t *quat, float x, float y, float z);
void luft_quat_mul(luft_quat_t *a, luft_quat_t *b, luft_quat_t *out);
void luft_quat_dup(luft_quat_t *in, luft_quat_t *out);
void luft_quat_to_matrix(luft_quat_t *quat, float m[16]);

#ifdef __cplusplus
}
#endif

#endif /* LUFTBALLONS_QUAT_H */

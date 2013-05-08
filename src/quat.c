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

#include <string.h>
#include <math.h>

#include "quat.h"

/**
 * Normalize a quaternion
 **/
static void
quat_normalize(quat_t *quat)
{
	float mag;

	mag = quat->c[0] * quat->c[0] +
	      quat->c[1] * quat->c[1] +
	      quat->c[2] * quat->c[2] +
	      quat->c[3] * quat->c[3];

	mag = sqrtf(mag);

	quat->c[0] /= mag;
	quat->c[1] /= mag;
	quat->c[2] /= mag;
	quat->c[3] /= mag;

	quat->mul_count = 0;
}

/**
 * Initialize a quaternion
 **/
void
quat_init(quat_t *quat, float x, float y, float z, float theta)
{
	float s = sinf(theta / 2);

	quat->c[0] = x * s;
	quat->c[1] = y * s;
	quat->c[2] = z * s;
	quat->c[3] = cosf(theta / 2);
	quat_normalize(quat);
}
EXPORT(quat_init);

/**
 * Multiply two quaternions
 **/
void
quat_mul(quat_t *a, quat_t *b, quat_t *out)
{
	quat_t *c = out;
	quat_t tmp;

	if (a == c || b == c)
		c = &tmp;

	c->c[0] = a->c[3] * b->c[0] +
		  a->c[0] * b->c[3] +
		  a->c[1] * b->c[2] -
		  a->c[2] * b->c[1];
	c->c[1] = a->c[3] * b->c[1] +
		  a->c[1] * b->c[3] +
		  a->c[2] * b->c[0] -
		  a->c[0] * b->c[2];
	c->c[2] = a->c[3] * b->c[2] +
		  a->c[2] * b->c[3] +
		  a->c[0] * b->c[1] -
		  a->c[1] * b->c[0];
	c->c[3] = a->c[3] * b->c[3] -
		  a->c[0] * b->c[0] -
		  a->c[1] * b->c[1] -
		  a->c[2] * b->c[2];

	c->mul_count = a->mul_count + b->mul_count + 1;

	if (c->mul_count > 10)
		quat_normalize(c);

	if (a == out || b == out)
		quat_dup(c, out);
}
EXPORT(quat_mul);

/**
 * Copy one quaternion into another.
 **/
void
quat_dup(quat_t *in, quat_t *out)
{
	memcpy(out, in, sizeof(quat_t));
}
EXPORT(quat_dup);

/**
 * Convert a quaternion to a matrix.
 **/
void
quat_to_matrix(quat_t *quat, float m[16])
{
	float w = quat->c[3];
	float x = quat->c[0];
	float y = quat->c[1];
	float z = quat->c[2];

	quat_normalize(quat);

	m[0]  = 1 - 2 * y * y - 2 * z * z;
	m[4]  =     2 * x * y - 2 * w * z;
	m[8]  =     2 * x * z + 2 * w * y;
	m[12] = 0;

	m[1]  =     2 * x * y + 2 * w * z;
	m[5]  = 1 - 2 * x * x - 2 * z * z;
	m[9]  =     2 * y * z - 2 * w * x;
	m[13] = 0;

	m[2]  =     2 * x * z - 2 * w * y;
	m[6]  =     2 * y * z + 2 * w * x;
	m[10] = 1 - 2 * x * x - 2 * y * y;
	m[14] = 0;

	m[3] = m[7] = m[11] = 0;
	m[15] = 1;
}
EXPORT(quat_to_matrix);

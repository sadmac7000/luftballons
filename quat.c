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

#include <string.h>
#include <math.h>

#include "quat.h"

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
	quat->c[3] = cosf(theta);
	quat->mul_count = 0;
}

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
}

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

/**
 * Copy one quaternion into another.
 **/
void
quat_dup(quat_t *in, quat_t *out)
{
	memcpy(out, in, sizeof(quat_t));
}

/**
 * Convert a quaternion to a matrix.
 **/
void
quat_to_matrix(quat_t *quat, float m[16])
{
	m[0]  = 1 - 2 * quat->c[1] * quat->c[1] - 2 * quat->c[2] * quat->c[2];
	m[1]  =     2 * quat->c[0] * quat->c[1] - 2 * quat->c[3] * quat->c[2];
	m[2]  =     2 * quat->c[0] * quat->c[2] + 2 * quat->c[3] * quat->c[1];
	m[3]  = 0;
	m[4]  =     2 * quat->c[0] * quat->c[1] + 2 * quat->c[3] * quat->c[2];
	m[5]  = 1 - 2 * quat->c[0] * quat->c[0] - 2 * quat->c[2] * quat->c[2];
	m[6]  =     2 * quat->c[1] * quat->c[2] - 2 * quat->c[3] * quat->c[0];
	m[7]  = 0;
	m[8]  =     2 * quat->c[0] * quat->c[2] - 2 * quat->c[3] * quat->c[1];
	m[9]  =     2 * quat->c[1] * quat->c[2] + 2 * quat->c[3] * quat->c[0];
	m[10] = 1 - 2 * quat->c[0] * quat->c[0] - 2 * quat->c[1] * quat->c[1];
	m[11] = m[12] = m[13] = m[14] = 0;
	m[15] = 1;
}

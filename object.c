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

#include "object.h"
#include "util.h"
#include "matrix.h"
#include "quat.h"

/**
 * Create an object.
 **/
object_t *
object_create(mesh_t *mesh)
{
	object_t *ret = xmalloc(sizeof(mesh_t));

	ret->mesh = mesh;
	mesh_grab(mesh);

	ret->trans[0] = ret->trans[1] = ret->trans[2] = 0;
	quat_init(&ret->rot, 0, 1, 0, 0);

	ret->children = NULL;
	ret->child_count = 0;

	return ret;
}

/**
 * Draw an object in the current context.
 **/
void
object_draw(object_t *object)
{
	size_t i;

	mesh_draw(object->mesh);
	
	/* FIXME: Recursion: Bad? */
	for (i = 0; i < object->child_count; i++)
		object_draw(object->children[i]);
}

/**
 * Rotate this object by the given quaternion.
 **/
void
object_rotate(object_t *object, quat_t *quat)
{
	quat_mul(quat, &object->rot, &object->rot);
}

/**
 * Translate this object by the given vector.
 **/
void
object_move(object_t *object, float vec[3])
{
	vec3_add(object->trans, vec, object->trans);
}

/**
 * Set this object's rotation.
 **/
void
object_set_rotation(object_t *object, quat_t *quat)
{
	quat_dup(quat, &object->rot);
}

/**
 * Set this object's translation.
 **/
void
object_set_translation(object_t *object, float vec[3])
{
	vec3_dup(vec, object->trans);
}

/**
 * Get a translation matrix for this object.
 **/
void
object_get_transform_mat(object_t *object, float matrix[16])
{
	float translate[16] = {
		1, 0, 0, object->trans[0],
		0, 1, 0, object->trans[1],
		0, 0, 1, object->trans[2],
		0, 0, 0, 1,
	};
	float rotate[16];

	quat_to_matrix(&object->rot, rotate);

	matrix_multiply(translate, rotate, matrix);
}

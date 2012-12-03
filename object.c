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

#include <err.h>
#include <string.h>

#include "object.h"
#include "util.h"
#include "matrix.h"
#include "quat.h"

/**
 * Find the object under this object that matches the given name.
 *
 * object: Object to search under.
 * name: Regex to check names against.
 **/
object_t *
object_lookup(object_t *object, const char *name)
{
	size_t i;
	object_t *ret;

	if (! strcmp(object->name, name))
		return object;

	for (i = 0; i < object->child_count; i++) {
		ret = object_lookup(object->children[i], name);

		if (ret)
			return ret;
	}

	return NULL;
}

/**
 * Set the value of the pretransform matrix.
 **/
void
object_apply_pretransform(object_t *object, float matrix[16])
{
	memcpy(object->pretransform, matrix, 16 * sizeof(float));
}

/**
 * Create an object.
 **/
object_t *
object_create(mesh_t *mesh, object_t *parent, material_t *material)
{
	object_t *ret = xmalloc(sizeof(object_t));
	MATRIX_DECL_IDENT(ident);

	ret->parent = parent;
	ret->mesh = mesh;
	ret->material = material;
	ret->name = NULL;

	if (mesh)
		mesh_grab(mesh);

	ret->trans[0] = ret->trans[1] = ret->trans[2] = 0;
	ret->scale[0] = ret->scale[1] = ret->scale[2] = 1;
	quat_init(&ret->rot, 0, 1, 0, 0);

	ret->children = NULL;
	ret->child_count = 0;

	if (ret->parent)
		object_add_child(ret->parent, ret);

	object_apply_pretransform(ret, ident);

	return ret;
}

/**
 * Set an object's name.
 **/
void
object_set_name(object_t *object, const char *name)
{
	free(object->name);
	object->name = xstrdup(name);
}

/**
 * Destroy an object.
 **/
void
object_destroy(object_t *object)
{
	object_unparent(object);

	/* FIXME: Recursion: Bad? */
	while (object->child_count)
		object_destroy(object->children[0]);

	if (object->mesh)
		mesh_ungrab(object->mesh);

	free(object->children);
	free(object);
}

/**
 * Draw an object given a global transform.
 **/
static void
object_draw_matrix(object_t *object, size_t pass, float parent_trans[16])
{
	size_t i;
	float transform[16];

	object_get_transform_mat(object, transform);
	matrix_multiply(parent_trans, transform, transform);

	if (object->mesh && object->material &&
	    material_activate(object->material, pass)) {
		shader_set_uniform_mat(object->material->shaders[pass],
				       "transform", transform);
		mesh_draw(object->mesh);
	}

	/* FIXME: Recursion: Bad? */
	for (i = 0; i < object->child_count; i++)
		object_draw_matrix(object->children[i], pass, transform);
}

/**
 * Draw an object in the current context.
 **/
void
object_draw(object_t *object, size_t pass, camera_t *camera)
{
	object_draw_matrix(object, pass, camera->to_clip_xfrm);
}

/**
 * Scale this object by the given XYZ scale factors.
 **/
void
object_scale(object_t *object, float scale[3])
{
	object->scale[0] *= scale[0];
	object->scale[1] *= scale[1];
	object->scale[2] *= scale[2];
}

/**
 * Set scale for this object to the given XYZ scale factors.
 **/
void
object_set_scale(object_t *object, float scale[3])
{
	object->scale[0] = scale[0];
	object->scale[1] = scale[1];
	object->scale[2] = scale[2];
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
	MATRIX_DECL(translate,
		    object->scale[0], 0, 0, object->trans[0],
		    0, object->scale[1], 0, object->trans[1],
		    0, 0, object->scale[2], object->trans[2],
		    0, 0, 0, 1);
	float rotate[16];
	float translate_final[16];

	quat_to_matrix(&object->rot, rotate);

	matrix_multiply(translate, object->pretransform, translate_final);
	matrix_multiply(translate_final, rotate, matrix);
}

/**
 * Add a child to an object.
 **/
void
object_add_child(object_t *object, object_t *child)
{
	if (! object->child_count)
		object->children = xrealloc(object->children,
					    sizeof(object_t *));
	else if (! (object->child_count & (object->child_count - 1)))
		object->children = xrealloc(object->children,
					    2 * object->child_count *
					    sizeof(object_t *));

	object->children[object->child_count++] = child;
	child->parent = object;
}

/**
 * Remove an object from its parent.
 **/
void
object_unparent(object_t *object)
{
	size_t i;
	object_t *parent;

	if (! object->parent)
		return;

	parent = object->parent;
	object->parent = NULL;

	for (i = 0; i < parent->child_count; i++)
		if (parent->children[i] == object)
			break;

	if (i == parent->child_count)
		err(1, "Broken parent link for object");

	memcpy(&parent->children[i], &parent->children[i + 1],
	       (parent->child_count - i) * sizeof(object_t *));
}

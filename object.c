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
 * Set up a cursor to traverse objects rooted at the given object, and prep for
 * a pre-order iteration.
 *
 * Returns: The first object in the iteration.
 **/
object_t *
object_cursor_start_pre(object_cursor_t *cursor, object_t *root)
{
	object_cursor_init(cursor, root);

	if (root->child_count)
		object_cursor_down(cursor, 0);

	return root;
}

/**
 * Continue a cursor on a pre-order iteration of objects.
 *
 * Returns: The next item to iterate.
 **/
object_t *
object_cursor_next_pre(object_cursor_t *cursor)
{
	ssize_t next = 0;
	object_t *ret;

	if (! cursor->stack_size)
		return NULL;

	ret = cursor->current;

	while (next >= 0 && cursor->current->child_count < (size_t)next)
		next = object_cursor_up(cursor);

	if (next >= 0)
		object_cursor_down(cursor, next);

	return ret;
}

/**
 * Set up a cursor to traverse objects rooted at the given object.
 **/
void
object_cursor_init(object_cursor_t *cursor, object_t *root)
{
	cursor->stack = NULL;
	cursor->stack_size = 0;
	cursor->current = root;
}

/**
 * Move the cursor to the given child of its current position.
 **/
void
object_cursor_down(object_cursor_t *cursor, size_t child)
{
	if (child > cursor->current->child_count)
		errx(1, "Tried to get child %zu of object with %zu children",
		     child, cursor->current->child_count);

	cursor->stack = vec_expand(cursor->stack, cursor->stack_size);
	cursor->stack[cursor->stack_size++] = child;
	cursor->current = cursor->current->children[child];
}

/**
 * Move the cursor to the parent of its current position.
 *
 * Returns: The index of the child we previously occupied or -1 if we're at the
 * root.
 **/
ssize_t
object_cursor_up(object_cursor_t *cursor)
{
	size_t ret;

	if (cursor->stack_size == 0)
		return -1;

	ret = cursor->stack[--cursor->stack_size];
	cursor->stack = vec_contract(cursor->stack, cursor->stack_size);
	cursor->current = cursor->current->parent;
	return ret;
}

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
object_create(object_t *parent)
{
	object_t *ret = xmalloc(sizeof(object_t));
	MATRIX_DECL_IDENT(ident);

	ret->parent = NULL;
	ret->type = OBJ_NODE;
	ret->name = NULL;

	ret->trans[0] = ret->trans[1] = ret->trans[2] = 0;
	ret->scale[0] = ret->scale[1] = ret->scale[2] = 1;
	quat_init(&ret->rot, 0, 1, 0, 0);

	ret->children = NULL;
	ret->child_count = 0;

	if (parent)
		object_reparent(ret, parent);

	object_apply_pretransform(ret, ident);

	return ret;
}

/**
 * If an object is not of type OBJ_NODE, make it type OBJ_NODE, clearing out
 * its resources in the process.
 **/
static void
object_make_nodetype(object_t *object)
{
	if (object->type == OBJ_NODE)
		return;

	if (object->type == OBJ_MESH)
		mesh_ungrab(object->mesh);

	object->type = OBJ_NODE;
}

/**
 * Make this object a light.
 **/
void
object_make_light(object_t *object, float color[3])
{
	object_make_nodetype(object);
	object->type = OBJ_LIGHT;
	memcpy(object->light_color, color, 3 * sizeof(float));
}

/**
 * Set an object to mesh type and set its material.
 **/
void
object_set_mesh(object_t *object, mesh_t *mesh, material_t *material)
{
	object_make_nodetype(object);

	object->type = OBJ_MESH;
	object->mesh = mesh;
	object->material = material;

	mesh_grab(mesh);
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
	ssize_t up;
	object_cursor_t cursor;

	object_cursor_init(&cursor, object);

	for (;;) {
		while (cursor.current->child_count)
			object_cursor_down(&cursor, 0);

		while (! cursor.current->child_count) {
			object = cursor.current;
			up = object_cursor_up(&cursor);
			
			object_unparent(object);

			object_make_nodetype(object);

			free(object->children);
			free(object);

			if (up < 0)
			    return;
		}
	}
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

	matrix_multiply(translate, rotate, translate_final);
	matrix_multiply(translate_final, object->pretransform, matrix);
}

/**
 * Add a child to an object.
 **/
void
object_reparent(object_t *object, object_t *parent)
{
	object_unparent(object);

	parent->children = vec_expand(parent->children, parent->child_count);
	parent->children[parent->child_count++] = object;
	object->parent = parent;
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

	parent->children = vec_contract(parent->children, parent->child_count);
}

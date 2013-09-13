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

#include <err.h>
#include <string.h>
#include <math.h>

#include "object.h"
#include "util.h"
#include "matrix.h"
#include "quat.h"

/**
 * Metadata for a camera.
 **/
struct camera {
	float *to_clip_xfrm;
	float near;
	float far;
	float zoom;
	float fov_scale;
	float aspect;
};

object_t *
object_get_fs_quad(void)
{
	vbuf_fmt_t format = 0;
	mesh_t *mesh;
	object_t *object;
	float verts[] = {
		-1.0, -1.0, 0.0, 1.0,
		1.0, -1.0, 0.0, 1.0,
		1.0, 1.0, 0.0, 1.0,
		-1.0, 1.0, 0.0, 1.0,
	};
	uint16_t elems[] = { 0, 1, 2, 3 };

	vbuf_fmt_add(&format, "position", 4, GL_FLOAT);

	mesh = mesh_create(4, verts, 4, elems, format, GL_TRIANGLE_FAN);
	object = object_create(NULL);
	object_set_mesh(object, mesh);
	mesh_ungrab(mesh);

	return object;
}

/**
 * Release an object cursor's internal data.
 **/
void
object_cursor_release(object_cursor_t *cursor)
{
	free(cursor->stack);
}
EXPORT(object_cursor_release);

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
EXPORT(object_cursor_start_pre);

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

	if (cursor->current->child_count > 0) {
		object_cursor_down(cursor, 0);
		return ret;
	}

	do {
		next = object_cursor_up(cursor) + 1;
	} while (next > 0 && cursor->current->child_count <= (size_t)next);

	if (next > 0)
		object_cursor_down(cursor, next);

	return ret;
}
EXPORT(object_cursor_next_pre);

/**
 * Set a cursor so that the next call to next_pre will skip over the current
 * object's children.
 **/
void
object_cursor_skip_children_pre(object_cursor_t *cursor)
{
	while (cursor->current->child_count)
		object_cursor_down(cursor, cursor->current->child_count - 1);
}
EXPORT(object_cursor_skip_children_pre);

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
EXPORT(object_cursor_init);

/**
 * Move the cursor to the given child of its current position.
 **/
void
object_cursor_down(object_cursor_t *cursor, size_t child)
{
	if (child >= cursor->current->child_count)
		errx(1, "Tried to get child %zu of object with %zu children",
		     child, cursor->current->child_count);

	cursor->stack = vec_expand(cursor->stack, cursor->stack_size);
	cursor->stack[cursor->stack_size++] = child;
	cursor->current = cursor->current->children[child];
}
EXPORT(object_cursor_down);

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
EXPORT(object_cursor_up);

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

	/* FIXME: use cursors, not recursion */
	for (i = 0; i < object->child_count; i++) {
		ret = object_lookup(object->children[i], name);

		if (ret)
			return ret;
	}

	return NULL;
}
EXPORT(object_lookup);

/**
 * Invalidate the transform cache.
 **/
static void
object_invalidate_transform_cache(object_t *object)
{
	object_cursor_t cursor;

	object_foreach_pre(cursor, object) {
		if (! object->transform_cache)
			continue;

		free(object->transform_cache);
		object->transform_cache = NULL;
	}

	object_cursor_release(&cursor);
}

/**
 * Set the value of the pretransform matrix.
 **/
void
object_apply_pretransform(object_t *object, float matrix[16])
{
	object_invalidate_transform_cache(object);
	memcpy(object->pretransform, matrix, 16 * sizeof(float));
}
EXPORT(object_apply_pretransform);

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

	if (object->type == OBJ_CAMERA)
		free(object->camera);

	object->type = OBJ_NODE;
}

/**
 * Destructor for an object.
 **/
void
object_destructor(void *object_)
{
	object_t *object = object_;

	while (object->child_count)
		object_reparent(object->children[object->child_count - 1],
				NULL);

	free(object->children);

	/* The parent should hold a reference to us if it exists. */
	if (object->parent)
		errx(1, "Object unreferenced but still has parent.");

	if (object->meta && object->meta_destructor)
		object->meta_destructor(object->meta);

	free(object->name);
	free(object->transform_cache);
	free(object->private_transform);

	object_make_nodetype(object);

	free(object);
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
	ret->mat = NO_MATERIAL;
	ret->transform_cache = NULL;
	ret->private_transform = NULL;
	ret->meta = NULL;
	ret->meta_destructor = 0;

	ret->draw_distance = 0;
	ret->child_draw_distance = 0;

	ret->trans[0] = ret->trans[1] = ret->trans[2] = 0;
	ret->scale[0] = ret->scale[1] = ret->scale[2] = 1;
	quat_init(&ret->rot, 0, 1, 0, 0);

	ret->children = NULL;
	ret->child_count = 0;

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, object_destructor, ret);

	object_apply_pretransform(ret, ident);

	if (parent)
		object_reparent(ret, parent);

	return ret;
}
EXPORT(object_create);

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
EXPORT(object_make_light);

/**
 * Make this object a collision box.
 **/
void
object_make_box_collider(object_t *object, float l, float w, float h)
{
	object_make_nodetype(object);
	object->type = OBJ_COLLIDER_BOX;
	object->l = l;
	object->w = w;
	object->h = h;
}
EXPORT(object_make_box_collider);

/**
 * Make an object a collision sphere.
 **/
void
object_make_sphere_collider(object_t *object, float r)
{
	object_make_nodetype(object);
	object->type = OBJ_COLLIDER_SPHERE;
	object->r = r;
}
EXPORT(object_make_sphere_collider);

/**
 * Make an object a collision cylinder.
 **/
void
object_make_cylinder_collider(object_t *object, float r, float h)
{
	object_make_nodetype(object);
	object->type = OBJ_COLLIDER_CYLINDER;
	object->r = r;
	object->h = h;
}
EXPORT(object_make_cylinder_collider);

/**
 * Component of the support function for a sphere collider.
 **/
static void
object_support_component_sphere(object_t *object,
				float direction[3], float out[3])
{
	float trans[16];

	vec3_normalize(direction, direction);
	vec3_scale(direction, direction, object->r);

	object_get_transform_mat(object, trans);

	out[0] = trans[3];
	out[1] = trans[7];
	out[2] = trans[11];

	vec3_add(direction, out, out);
}

/**
 * Component of the support function for a cylinder collider.
 **/
static void
object_support_component_cylinder(object_t *object,
				  float direction[3], float out[3])
{
	float top[3] = { 0, object->h / 2, 0 };
	float bottom[3] = { 0, -object->h / 2, 0 };
	float axis[3];
	float dir_perp[3];
	float trans[16];
	float dot_top;

	object_get_transform_mat(object, trans);

	/* Top and bottom are set to points in the middle of the top and the
	 * bottom faces of the cylinder respectively. We transform them to
	 * their worldspace positions.
	 */
	matrix_vec3_mul(trans, top, 1.0, top);
	matrix_vec3_mul(trans, bottom, 1.0, bottom);

	/* We get an axis vector that runs down the middle of the cylinder. */
	vec3_subtract(top, bottom, axis);

	/* Part of another process, but useful now for a reason... */
	vec3_cross(axis, direction, dir_perp);

	/* If the cross product is zero, our direction is aligned with the
	 * cylinder and top and bottom are our two final candidates.
	 */
	if (vec3_magnitude(dir_perp) > 0) {
		/* This completes what we started with the last cross product.
		 * dir_perp is now a vector perpendicular to the cylinder's
		 * axis, but as close to our selected direction as possible.
		 */
		vec3_cross(dir_perp, axis, dir_perp);

		/* Scale dir_perp to be the radius of our cylinder.
		 */
		vec3_normalize(dir_perp, dir_perp);
		vec3_scale(dir_perp, dir_perp, object->r);

		/* Finally, move top and bottom to the edges of our cylinder in
		 * the appropriate direction. We now have our two final
		 * candidates.
		 */
		vec3_add(dir_perp, top, top);
		vec3_add(dir_perp, bottom, bottom);
	}

	/* We now have two candidates, top and bottom. We can just use largest
	 * dot product to determine which is furthest.
	 */
	dot_top = vec3_dot(top, direction);

	if (dot_top > vec3_dot(bottom, direction))
		memcpy(out, top, 3 * sizeof(float));
	else
		memcpy(out, bottom, 3 * sizeof(float));
}

/**
 * Component of the support function for a cylinder collider.
 **/
static void
object_support_component_box(object_t *object,
			     float direction[3], float out[3])
{
	float x = object->w / 2;
	float y = object->h / 2;
	float z = object->l / 2;
	float trans[16];
	float dot;
	float max_val = -INFINITY;
	size_t max;
	size_t i;

	float coords[8][3] = {
		{ +x, +y, +z, },
		{ +x, +y, -z, },
		{ +x, -y, +z, },
		{ +x, -y, -z, },
		{ -x, +y, +z, },
		{ -x, +y, -z, },
		{ -x, -y, +z, },
		{ -x, -y, -z, },
	};

	object_get_transform_mat(object, trans);

	for (i = 0, max = 0; i < 8; i++) {
		matrix_vec3_mul(trans, coords[i], 1, coords[i]);
		dot = vec3_dot(direction, coords[i]);

		if (dot <= max_val)
			continue;

		max_val = dot;
		max = i;
	}

	memcpy(out, coords[max], 3 * sizeof(float));
}

/**
 * Component of the support function for a single object.
 **/
static void
object_support_component(object_t *object, float direction[3], float out[3])
{
	if (object->type == OBJ_COLLIDER_SPHERE)
		object_support_component_sphere(object, direction, out);
	else if (object->type == OBJ_COLLIDER_CYLINDER)
		object_support_component_cylinder(object, direction, out);
	else if (object->type == OBJ_COLLIDER_BOX)
		object_support_component_box(object, direction, out);
	else
		errx(1, "Only colliders have a support component");
}

/**
 * GJK collision support function.
 **/
static void
object_support(object_t *a, object_t *b, float direction[3], float out[3])
{
	float pa[3];
	float pb[3];

	object_support_component(a, direction, pa);
	vec3_scale(direction, direction, -1);
	object_support_component(b, direction, pb);
	vec3_subtract(pa, pb, out);
}

/**
 * Check whether an object is a collider.
 **/
static int
object_is_collider(object_t *object)
{
	if (object->type == OBJ_COLLIDER_SPHERE)
		return 1;
	if (object->type == OBJ_COLLIDER_CYLINDER)
		return 1;
	if (object->type == OBJ_COLLIDER_BOX)
		return 1;

	return 0;
}

/**
 * Figure out what region the origin is in relative to a collision simplex.
 * Returns a vertex index for the region opposite that vertex, 4 if we're
 * inside the simplex
 **/
static size_t
object_simplex_detect_region(float simplex[4][3], size_t simplex_pos)
{
	MATRIX_DECL(base_det_matrix,
		simplex[0][0], simplex[0][1], simplex[0][2], 1,
		simplex[1][0], simplex[1][1], simplex[1][2], 1,
		simplex[2][0], simplex[2][1], simplex[2][2], 1,
		simplex[3][0], simplex[3][1], simplex[3][2], 1);
	float boundary_det_matrices[4][16] = {
		{	0, simplex[1][0], simplex[2][0], simplex[3][0],
			0, simplex[1][1], simplex[2][1], simplex[3][1],
			0, simplex[1][2], simplex[2][2], simplex[3][2],
			1, 1, 1, 1,
		}, {	simplex[0][0], 0, simplex[2][0], simplex[3][0],
			simplex[0][1], 0, simplex[2][1], simplex[3][1],
			simplex[0][2], 0, simplex[2][2], simplex[3][2],
			1, 1, 1, 1,
		}, {	simplex[0][0], simplex[1][0], 0, simplex[3][0],
			simplex[0][1], simplex[1][1], 0, simplex[3][1],
			simplex[0][2], simplex[1][2], 0, simplex[3][2],
			1, 1, 1, 1,
		}, {	simplex[0][0], simplex[1][0], simplex[2][0], 0,
			simplex[0][1], simplex[1][1], simplex[2][1], 0,
			simplex[0][2], simplex[1][2], simplex[2][2], 0,
			1, 1, 1, 1,
		},
	};
	float base_det;
	size_t i;

	base_det = matrix_determinant(base_det_matrix);

	for (i = 0; i < 4; i++) {
		if (i == simplex_pos)
			continue;

		if (base_det *
		    matrix_determinant(boundary_det_matrices[i]) > 0)
			break;
	}

	return i;
}

/**
 * Check for collision between objects.
 **/
int
object_check_collision(object_t *a, object_t *b)
{
	float direction[3] = { 0, 1, 0 };
	float simplex[4][3];
	size_t simplex_pos = 5;
	size_t last;
	float middle[3];
	float to_last[3];

	if (! (object_is_collider(a) &&
	       object_is_collider(b)))
		return 0;

	object_support(a, b, direction, simplex[0]);

	direction[2] = -1;
	object_support(a, b, direction, simplex[1]);

	if (vec3_dot(simplex[1], direction) <= 0)
		return 0;

	vec3_subtract(simplex[1], simplex[0], to_last);
	vec3_cross(simplex[1], to_last, direction);
	vec3_cross(direction, to_last, direction);
	object_support(a, b, direction, simplex[2]);

	if (vec3_dot(simplex[2], direction) <= 0)
		return 0;

	while (vec3_dot(simplex[simplex_pos], direction)) {
		if (simplex_pos == 5) {
			simplex_pos = 3;
			last = 2;
		} else {
			last = simplex_pos;
			simplex_pos =
				object_simplex_detect_region(simplex,
							     simplex_pos);
		}

		if (simplex_pos == 4)
			return 1;

		middle[0] = middle[1] = middle[2] = 0;

		if (simplex_pos != 3)
			vec3_add(simplex[3], middle, middle);

		if (simplex_pos != 2)
			vec3_add(simplex[2], middle, middle);

		if (simplex_pos != 1)
			vec3_add(simplex[1], middle, middle);

		if (simplex_pos != 0)
			vec3_add(simplex[0], middle, middle);

		vec3_scale(middle, middle, 1.0/3.0);
		vec3_subtract(middle, simplex[last], to_last);
		vec3_cross(middle, to_last, direction);
		vec3_cross(direction, to_last, direction);
		object_support(a, b, direction, simplex[simplex_pos]);
	}

	return 0;
}
EXPORT(object_check_collision);

/**
 * Make this object a camera.
 **/
void
object_make_camera(object_t *object, float fov, float near, float far)
{
	fov *= 3.14159;
	fov /= 360;

	object_make_nodetype(object);
	object->type = OBJ_CAMERA;
	object->camera = xmalloc(sizeof(struct camera));
	object->camera->near = near;
	object->camera->far = far;
	object->camera->fov_scale = cosf(fov) / sinf(fov);
	object->camera->aspect = 1.0;
	object->camera->zoom = 1.0;
	object->camera->to_clip_xfrm = NULL;
}
EXPORT(object_make_camera);

/**
 * Invalidate the clip transform matrix.
 **/
static void
camera_invalidate_clip(object_t *camera)
{
	if (camera->camera->to_clip_xfrm)
		free(camera->camera->to_clip_xfrm);

	camera->camera->to_clip_xfrm = NULL;
}

/**
 * Get the clip matrix for this camera.
 **/
void
camera_to_clip(object_t *camera, float mat[16])
{
	float scale;
	float aspect;
	float near;
	float far;

	if (camera->type != OBJ_CAMERA)
		errx(1, "camera_to_clip must be called on "
		     "an object of type camera");

	scale = camera->camera->zoom * camera->camera->fov_scale;
	aspect = camera->camera->aspect;
	near = camera->camera->near;
	far = camera->camera->far;

	if (! camera->camera->to_clip_xfrm) {
		camera->camera->to_clip_xfrm = xcalloc(16, sizeof(float));
		camera->camera->to_clip_xfrm[0] = scale / aspect;
		camera->camera->to_clip_xfrm[5] = scale;
		camera->camera->to_clip_xfrm[10] = (near + far)/(near - far);
		camera->camera->to_clip_xfrm[11] = -1;
		camera->camera->to_clip_xfrm[14] = 2*near*far/(near - far);
	}

	memcpy(mat, camera->camera->to_clip_xfrm, 16 * sizeof(float));
}

/**
 * Get the cameraspace matrix for this camera.
 **/
void
camera_from_world(object_t *camera, float mat[16])
{
	if (camera->type != OBJ_CAMERA)
		errx(1, "camera_from_world must be called on "
		     "an object of type camera");

	/**
	 * We don't yet cache this. Necessary? Also there's the question of
	 * whether scaling is a problem here.
	 **/
	object_get_total_transform(camera, mat);
	matrix_transpose(mat, mat);
	matrix_inverse_trans(mat, mat);
}

/**
 * Set the aspect ratio of a camera.
 **/
void
camera_set_aspect(object_t *camera, float aspect)
{
	camera_invalidate_clip(camera);

	if (camera->type != OBJ_CAMERA)
		errx(1, "Setting view aspect ratio of an object "
		     "that isn't a camera.");

	camera->camera->aspect = aspect;
}
EXPORT(camera_set_aspect);

/**
 * Set an object to mesh type.
 **/
void
object_set_mesh(object_t *object, mesh_t *mesh)
{
	object_make_nodetype(object);

	object->type = OBJ_MESH;
	object->mesh = mesh;

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
EXPORT(object_set_name);

/**
 * Grab an object.
 **/
void
object_grab(object_t *object)
{
	refcount_grab(&object->refcount);
}
EXPORT(object_grab);

/**
 * Ungrab an object.
 **/
void
object_ungrab(object_t *object)
{
	refcount_ungrab(&object->refcount);
}
EXPORT(object_ungrab);

/**
 * Scale this object by the given XYZ scale factors.
 **/
void
object_scale(object_t *object, float scale[3])
{
	object_invalidate_transform_cache(object);
	object->scale[0] *= scale[0];
	object->scale[1] *= scale[1];
	object->scale[2] *= scale[2];
}
EXPORT(object_scale);

/**
 * Set scale for this object to the given XYZ scale factors.
 **/
void
object_set_scale(object_t *object, float scale[3])
{
	object_invalidate_transform_cache(object);
	object->scale[0] = scale[0];
	object->scale[1] = scale[1];
	object->scale[2] = scale[2];
}
EXPORT(object_set_scale);

/**
 * Rotate this object by the given quaternion.
 **/
void
object_rotate(object_t *object, quat_t *quat)
{
	object_invalidate_transform_cache(object);
	quat_mul(quat, &object->rot, &object->rot);
}
EXPORT(object_rotate);

/**
 * Translate this object by the given vector.
 **/
void
object_move(object_t *object, float vec[3])
{
	object_invalidate_transform_cache(object);
	vec3_add(object->trans, vec, object->trans);
}
EXPORT(object_move);

/**
 * Set this object's rotation.
 **/
void
object_set_rotation(object_t *object, quat_t *quat)
{
	object_invalidate_transform_cache(object);
	quat_dup(quat, &object->rot);
}
EXPORT(object_set_rotation);

/**
 * Set this object's translation.
 **/
void
object_set_translation(object_t *object, float vec[3])
{
	object_invalidate_transform_cache(object);
	vec3_dup(vec, object->trans);
}
EXPORT(object_set_translation);

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
EXPORT(object_get_transform_mat);

/**
 * Create the transform cache.
 **/
static void
object_fill_transform_cache(object_t *object)
{
	if (object->transform_cache)
		return;

	object->transform_cache = xcalloc(16, sizeof(float));
	object_get_transform_mat(object, object->transform_cache);

	if (! object->parent)
		return;

	object_fill_transform_cache(object->parent);
	matrix_multiply(object->parent->transform_cache,
			object->transform_cache, object->transform_cache);
}

/**
 * Get a transform matrix for this object including the transform of its
 * parents.
 **/
void
object_get_total_transform(object_t *object, float mat[16])
{
	object_fill_transform_cache(object);

	if (! object->private_transform)
		memcpy(mat, object->transform_cache, 16 * sizeof(float));
	else
		matrix_multiply(object->transform_cache,
				object->private_transform, mat);
}
EXPORT(object_get_total_transform);

/**
 * Find the distance between two objects.
 **/
float
object_distance(object_t *a, object_t *b)
{
	float a_trans[16];
	float b_trans[16];
	float x, y, z;

	object_get_total_transform(a, a_trans);
	object_get_total_transform(b, b_trans);

	x = a_trans[13] - b_trans[13];
	y = a_trans[14] - b_trans[14];
	z = a_trans[15] - b_trans[15];

	return sqrtf(x * x + y * y + z * z);
}
EXPORT(object_distance);

/**
 * Remove an object from its parent. This will ungrab the object and MAY FREE
 * IT.
 **/
static void
object_unparent(object_t *object)
{
	size_t i;
	object_t *parent;

	if (! object->parent)
		return;

	object_invalidate_transform_cache(object);
	parent = object->parent;
	object->parent = NULL;

	for (i = 0; i < parent->child_count; i++)
		if (parent->children[i] == object)
			break;

	if (i == parent->child_count)
		err(1, "Broken parent link for object");

	parent->children = vec_del(parent->children, parent->child_count, i);
	parent->child_count--;
	object_ungrab(object);
}

/**
 * Add a child to an object. If the parent is NULL then we unparent the object,
 * which MAY FREE THE OBJECT.
 **/
void
object_reparent(object_t *object, object_t *parent)
{
	object_grab(object);
	object_unparent(object);

	object->parent = parent;

	if (parent) {
		parent->children = vec_expand(parent->children, parent->child_count);
		parent->children[parent->child_count++] = object;
	} else {
		object_ungrab(object);
	}
}
EXPORT(object_reparent);

/**
 * Set an object's material.
 **/
void
object_set_material(object_t *object, material_t mat)
{
	object->mat = mat;
}
EXPORT(object_set_material);

/**
 * Set draw distance for this object, but not its children.
 **/
void
object_set_draw_distance_local(object_t *object, float dist)
{
	object->draw_distance = dist;
}
EXPORT(object_set_draw_distance_local);

/**
 * Set draw distance for this object's children.
 **/
void
object_set_draw_distance_children(object_t *object, float dist)
{
	object->child_draw_distance = dist;
}
EXPORT(object_set_draw_distance_children);

/**
 * Set draw distance for this object.
 **/
void
object_set_draw_distance(object_t *object, float dist)
{
	object_set_draw_distance_local(object, dist);
	object_set_draw_distance_children(object, dist);
}
EXPORT(object_set_draw_distance);

/**
 * Get an object's name.
 **/
const char *
object_get_name(object_t *object)
{
	return object->name;
}
EXPORT(object_get_name);

/**
 * Get the type of an object.
 **/
object_type_t
object_get_type(object_t *object)
{
	return object->type;
}
EXPORT(object_get_type);

/**
 * Set an object's metadata.
 **/
void
object_set_meta(object_t *object, void *meta, void (*meta_destructor)(void *))
{
	if (object->meta && object->meta_destructor)
		object->meta_destructor(object->meta);

	object->meta = meta;
	object->meta_destructor = meta_destructor;
}
EXPORT(object_set_meta);

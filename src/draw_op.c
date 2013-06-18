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

#include "draw_op.h"
#include "matrix.h"
#include "bufpool.h"
#include "mesh.h"
#include "shader.h"

bufpool_t **pools;
size_t num_pools;

/**
 * Destroy a draw operation.
 **/
static void
draw_op_destructor(void *op_)
{
	draw_op_t *op = op_;

	object_ungrab(op->object);
	object_ungrab(op->camera);

	if (op->state)
		state_ungrab(op->state);

	free(op);
}

/**
 * Create a new draw operation.
 **/
draw_op_t *
draw_op_create(object_t *object, object_t *camera, state_t *state)
{
	draw_op_t *ret = xcalloc(1, sizeof(draw_op_t));

	object_grab(object);
	object_grab(camera);

	if (state)
		state_grab(state);

	ret->object = object;
	ret->camera = camera;
	ret->state = state;

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, draw_op_destructor, ret);

	return ret;
}
EXPORT(draw_op_create);

/**
 * Grab a Operation.
 **/
void
draw_op_grab(draw_op_t *op)
{
	refcount_grab(&op->refcount);
}
EXPORT(draw_op_grab);

/**
 * Ungrab a Operation.
 **/
void
draw_op_ungrab(draw_op_t *op)
{
	refcount_ungrab(&op->refcount);
}
EXPORT(draw_op_ungrab);

/**
 * Add a mesh to our array of bufpools.
 **/
static void
draw_op_add_mesh(mesh_t *mesh)
{
	size_t i;
	bufpool_t *pool;

	for (i = 0; i < num_pools; i++) {
		if (pools[i]->format != mesh->format)
			continue;

		bufpool_add_mesh(pools[i], mesh);
		return;
	}

	pools = vec_expand(pools, num_pools);

	pool = bufpool_create(mesh->format);
	bufpool_add_mesh(pool, mesh);
	pools[num_pools++] = pool;
}

/**
 * Draw a single object.
 *
 * Returns: true on success.
 **/
static int
draw_op_do_draw(object_t *object, float cspace[16],
		   float clip[16], object_t *quad)
{
	uniform_t *un;
	float trans[16];
	float fl[16];

	if (! state_material_active(object->mat_id))
		return 1;

	if (object->type == OBJ_NODE)
		return 1;

	if (object->type == OBJ_CAMERA)
		return 1;

	object_get_total_transform(object, trans);

	matrix_multiply(cspace, trans, fl);
	un = uniform_create(UNIFORM_MAT4, "transform", fl);
	shader_set_temp_uniform(un);
	uniform_ungrab(un);

	un = uniform_create(UNIFORM_MAT4, "clip_transform", clip);
	shader_set_temp_uniform(un);
	uniform_ungrab(un);

	if (object->type == OBJ_MESH) {
		draw_op_add_mesh(object->mesh);
		return mesh_draw(object->mesh);
	}

	memcpy(fl, object->light_color, 3 * sizeof(float));
	fl[3] = 1;
	un = uniform_create(UNIFORM_VEC4, "light_color", fl);
	shader_set_temp_uniform(un);
	uniform_ungrab(un);

	draw_op_add_mesh(quad->mesh);
	return mesh_draw(quad->mesh);
}

/**
 * Perform a given draw operation
 **/
void
draw_op_exec(draw_op_t *op)
{
	float cspace[16];
	float clip[16];
	float distance;
	object_t **flat = NULL;
	size_t num_flat = 0;
	size_t i;
	size_t j;
	object_cursor_t cursor;
	object_t *quad = object_get_fs_quad();
	object_t *object = op->object;

	camera_from_world(op->camera, cspace);
	camera_to_clip(op->camera, clip);

	object_foreach_pre(cursor, object) {
		distance = object_distance(object, op->camera);

		if (object->draw_distance == 0 ||
		    distance <= object->draw_distance) {
			flat = vec_expand(flat, num_flat);
			flat[num_flat++] = object;
		}

		if (object->child_draw_distance > 0 &&
		    distance > object->child_draw_distance)
			pre_skip_children(&cursor);
	}

	object_cursor_release(&cursor);

	if (op->state)
		state_push(op->state);

	while (num_flat) {
		for (i = 0; i < num_pools; i++)
			bufpool_end_generation(pools[i]);

		for (i = 0, j = 0; i < num_flat; i++) {
			flat[j] = flat[i];

			if (! draw_op_do_draw(flat[j], cspace, clip, quad))
				j++;
		}

		num_flat = j;
	}

	if (op->state)
		state_pop(op->state);

	object_ungrab(quad);

	free(flat);
}
EXPORT(draw_op_exec);
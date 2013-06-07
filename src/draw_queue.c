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

#include "draw_queue.h"
#include "state.h"
#include "shader.h"
#include "matrix.h"

/**
 * Create a new draw queue.
 **/
draw_queue_t *
draw_queue_create(void)
{
	draw_queue_t *queue = xmalloc(sizeof(draw_queue_t));

	queue->pool_count = 0;
	queue->pools = NULL;

	return queue;
}

/**
 * Add a mesh to this draw queue.
 **/
static void
draw_queue_add_mesh(draw_queue_t *queue, mesh_t *mesh)
{
	size_t i;
	bufpool_t *pool;

	for (i = 0; i < queue->pool_count; i++) {
		if (queue->pools[i]->format != mesh->format)
			continue;

		bufpool_add_mesh(queue->pools[i], mesh);
		return;
	}

	queue->pools = vec_expand(queue->pools, queue->pool_count);

	pool = bufpool_create(mesh->format);
	bufpool_add_mesh(pool, mesh);
	queue->pools[queue->pool_count++] = pool;
}

/**
 * Draw a single object.
 *
 * Returns: true on success.
 **/
static int
draw_queue_do_draw(draw_queue_t *queue, object_t *object, float cspace[16],
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
	un = uniform_create("transform", UNIFORM_MAT4, fl);
	shader_set_temp_uniform(un);
	uniform_ungrab(un);

	un = uniform_create("clip_transform", UNIFORM_MAT4, clip);
	shader_set_temp_uniform(un);
	uniform_ungrab(un);

	if (object->type == OBJ_MESH) {
		draw_queue_add_mesh(queue, object->mesh);
		return mesh_draw(object->mesh);
	}

	memcpy(fl, object->light_color, 3 * sizeof(float));
	fl[3] = 1;
	un = uniform_create("light_color", UNIFORM_VEC4, fl);
	shader_set_temp_uniform(un);
	uniform_ungrab(un);

	draw_queue_add_mesh(queue, quad->mesh);
	return mesh_draw(quad->mesh);
}

/**
 * Draw a given object from the perspective of the given camera.
 **/
void
draw_queue_draw(draw_queue_t *queue, object_t *object, object_t *camera)
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

	camera_from_world(camera, cspace);
	camera_to_clip(camera, clip);

	object_foreach_pre(cursor, object) {
		distance = object_distance(object, camera);

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

	while (num_flat) {
		for (i = 0; i < queue->pool_count; i++)
			bufpool_end_generation(queue->pools[i]);

		for (i = 0, j = 0; i < num_flat; i++) {
			flat[j] = flat[i];

			if (! draw_queue_do_draw(queue, flat[j], cspace, clip,
						 quad))
				j++;
		}

		num_flat = j;
	}

	object_ungrab(quad);

	free(flat);
}

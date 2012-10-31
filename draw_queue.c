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

#include "draw_queue.h"

/**
 * Uniform data for a draw op.
 **/
struct uniform {
	const char *name;
	enum {
		MAT4,
	} type;
	void *data;
};

/**
 * A draw operation. More or less corresponds to a single draw call.
 **/
struct draw_op {
	list_node_t link;
	shader_t *shader;
	mesh_t *mesh;
	struct uniform *uniforms;
	size_t uniform_count;
};

/**
 * Create a new draw queue.
 **/
draw_queue_t *
draw_queue_create(void)
{
	draw_queue_t *queue = xmalloc(sizeof(draw_queue_t));

	queue->pool_count = 0;
	queue->pools = NULL;
	
	list_init(&queue->draw_ops);

	return queue;
}

/**
 * Add a mesh to this draw queue.
 **/
static void
draw_queue_add_mesh(draw_queue_t *queue, mesh_t *mesh)
{
	size_t i;
	size_t alloc_size = 8;
	bufpool_t *pool;

	for (i = 0; i < queue->pool_count; i++) {
		if (queue->pools[i]->format != mesh->format)
			continue;

		bufpool_add_mesh(queue->pools[i], mesh);
		return;
	}

	while (alloc_size < queue->pool_count)
		alloc_size <<= 1;

	if (!queue->pools || alloc_size == queue->pool_count)
		queue->pools = xrealloc(queue->pools,
					sizeof(bufpool_t *) * alloc_size);

	pool = bufpool_create(mesh->format);
	bufpool_add_mesh(pool, mesh);
	queue->pools[queue->pool_count++] = pool;
}

/**
 * Add a draw op to the draw queue.
 *
 * queue: The queue to add to.
 * shader: The shader to perform the draw with.
 * mesh: The mesh to draw.
 * transform: The transform to apply to the mesh.
 **/
static void
draw_queue_add_op(draw_queue_t *queue, shader_t *shader, mesh_t *mesh,
		  float transform[16])
{
	struct draw_op *op = xmalloc(sizeof(struct draw_op));

	op->shader = shader;
	op->mesh = mesh;
	op->uniforms = xmalloc(sizeof(struct uniform));
	op->uniforms->name = "transform";
	op->uniforms->type = MAT4;
	op->uniforms->data = xcalloc(16, sizeof(float));
	memcpy(op->uniforms->data, transform, 16 * sizeof(float));
	op->uniform_count = 1;

	list_init(&op->link);
	list_insert(&queue->draw_ops, &op->link);

	draw_queue_add_mesh(queue, mesh);
}

/**
 * Queue a draw operation for the given object. Use the given shader and apply
 * the given transform.
 **/
static void
draw_queue_draw_matrix(draw_queue_t *queue, object_t *object, shader_t *shader,
		       float parent_trans[16])
{
	size_t i;
	float transform[16];

	object_get_transform_mat(object, transform);
	matrix_multiply(parent_trans, transform, transform);

	if (object->mesh)
		draw_queue_add_op(queue, shader, object->mesh, transform);

	/* FIXME: Recursion: Bad? */
	for (i = 0; i < object->child_count; i++)
		draw_queue_draw_matrix(queue, object->children[i], shader,
				       transform);
}

/**
 * Queue a draw operation for the given object. Use the given shader and camera.
 **/
void
draw_queue_draw(draw_queue_t *queue, object_t *object, shader_t *shader,
		camera_t *camera)
{
	draw_queue_draw_matrix(queue, object, shader, camera->to_clip_xfrm);
}

/**
 * Flush this draw queue.
 **/
void
draw_queue_flush(draw_queue_t *queue)
{
	size_t i;
	size_t j;
	size_t count = 0;
	struct draw_op **ops;

	for (i = 0; i < queue->pool_count; i++)
		bufpool_end_generation(queue->pools[i]);

	foreach(&queue->draw_ops)
		count++;

	if (! count)
		return;

	ops = xcalloc(count, sizeof(struct draw_op *));

	i = 0;
	foreach(&queue->draw_ops)
		ops[i++] = CONTAINER_OF(pos, struct draw_op, link);

	for (j = 0; j < count; j++) {
		shader_activate(ops[j]->shader);

		for (i = 0; i < ops[j]->uniform_count; i++) {
			if (ops[j]->uniforms[i].type != MAT4)
				continue;

			shader_set_uniform_mat(ops[j]->shader,
					       ops[j]->uniforms[i].name,
					       ops[j]->uniforms[i].data);
		}

		mesh_draw(ops[j]->mesh);

		list_remove(&ops[j]->link);
		free(ops[j]->uniforms);
		free(ops[j]);
	}

	free(ops);
}

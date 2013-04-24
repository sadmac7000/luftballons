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
#include "state.h"
#include "shader.h"

/**
 * A draw operation. More or less corresponds to a single draw call.
 **/
struct draw_op {
	mesh_t *mesh;
	uniform_t **uniforms;
	size_t uniform_count;
	int mat_id;
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
	
	queue->flags = 0;
	queue->draw_ops = NULL;
	queue->draw_op_count = 0;

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
 * Add a draw op to the draw queue.
 *
 * queue: The queue to add to.
 * object: The object contianing the mesh to draw.
 * transform: The transform to apply to the mesh.
 * camera: The camera we are drawing with.
 **/
static void
draw_queue_add_op(draw_queue_t *queue, object_t *object,
		  float transform[16], camera_t *camera)
{
	struct draw_op *op = xmalloc(sizeof(struct draw_op));
	float normal_trans[16];
	uniform_value_t tmp;

	queue->draw_ops = vec_expand(queue->draw_ops, queue->draw_op_count);

	memcpy(normal_trans, transform, 16 * sizeof(float));
	normal_trans[12] = normal_trans[13] = normal_trans[14] = 0;
	matrix_inverse_trans(normal_trans, normal_trans);

	op->mesh = object->mesh;
	op->mat_id = object->mat_id;
	op->uniforms = xcalloc(3, sizeof(struct uniform *));

	tmp.data_ptr = xcalloc(16, sizeof(float));
	memcpy(tmp.data_ptr, transform, 16 * sizeof(float));
	op->uniforms[0] = uniform_create("transform", UNIFORM_MAT4, tmp);
	uniform_grab(op->uniforms[0]);

	tmp.data_ptr = xcalloc(16, sizeof(float));
	memcpy(tmp.data_ptr, normal_trans, 16 * sizeof(float));
	op->uniforms[1] = uniform_create("normal_transform", UNIFORM_MAT4, tmp);
	uniform_grab(op->uniforms[1]);

	tmp.data_ptr = xcalloc(16, sizeof(float));
	memcpy(tmp.data_ptr, camera->to_clip_xfrm, 16 * sizeof(float));
	op->uniforms[2] = uniform_create("clip_transform", UNIFORM_MAT4, tmp);
	uniform_grab(op->uniforms[2]);

	op->uniform_count = 3;

	draw_queue_add_mesh(queue, object->mesh);

	queue->draw_ops[queue->draw_op_count++] = op;
}

/**
 * Add a light to this draw queue.
 **/
static void
draw_queue_add_light(draw_queue_t *queue, object_t *object,
		     float transform[16])
{
	(void)queue;
	(void)object;
	(void)transform;
	return;
}

/**
 * Queue a draw operation for the given object. Draw and
 * apply the given transform.
 **/
static void
draw_queue_draw_matrix(draw_queue_t *queue, object_t *object,
		       float parent_trans[16], camera_t *camera)
{
	ssize_t i = 0;
	float transform[16];
	object_cursor_t cursor;
	MATRIX_STACK_DECL(pt_stack);

	object_cursor_init(&cursor, object);
	object = cursor.current;

	for (;;) {
		object_get_transform_mat(object, transform);
		matrix_multiply(parent_trans, transform, transform);

		if (object->type == OBJ_MESH)
			draw_queue_add_op(queue, object, transform, camera);
		else if (object->type == OBJ_LIGHT)
			draw_queue_add_light(queue, object, transform);

		for (;(size_t)i >= object->child_count; i++) {
			i = object_cursor_up(&cursor);

			if (i < 0) {
				object_cursor_release(&cursor);
				matrix_stack_release(&pt_stack);
				return;
			}

			matrix_dup(parent_trans, transform);
			matrix_stack_pop(&pt_stack, parent_trans);

			object = cursor.current;
		}

		object_cursor_down(&cursor, i);
		i = 0;
		object = cursor.current;
		matrix_stack_push(&pt_stack, parent_trans);
		matrix_dup(transform, parent_trans);
	}
}

/**
 * Queue a draw operation for the given object. Draw from
 * the perspective of the given camera.
 **/
void
draw_queue_draw(draw_queue_t *queue, object_t *object, camera_t *camera)
{
	draw_queue_draw_matrix(queue, object, camera->to_cspace_xfrm, camera);
}

/**
 * Execute a draw operation.
 **/
static int
draw_queue_exec_op(struct draw_op *op)
{
	size_t i;

	if (! state_material_active(op->mat_id))
		return 0;

	for (i = 0; i < op->uniform_count; i++)
		shader_set_temp_uniform(op->uniforms[i]);

	return mesh_draw(op->mesh);
}

/**
 * Do as many of the draw operations as we can. Leave the rest in the queue.
 **/
static size_t
draw_queue_try_flush(draw_queue_t *queue)
{
	size_t i;
	size_t j;
	size_t k;
	struct draw_op *op;

	for (i = 0; i < queue->pool_count; i++)
		bufpool_end_generation(queue->pools[i]);

	i = 0;
	for (j = 0; j < queue->draw_op_count; j++) {
		queue->draw_ops[i] = queue->draw_ops[j];
		op = queue->draw_ops[i];

		if (draw_queue_exec_op(op)) {
			for (k = 0; k < op->uniform_count; k++)
				uniform_ungrab(op->uniforms[k]);
			free(op->uniforms);
			free(op);
		} else {
			i++;
		}
	}

	queue->draw_op_count = i;
	return queue->draw_op_count;
}

/**
 * Flush this draw queue.
 **/
void
draw_queue_flush(draw_queue_t *queue)
{
	int flags = 0;
	int retry = 1;
	size_t i;

	if (queue->flags & DRAW_QUEUE_CLEAR) {
		glClearColor(queue->clear_color[0], queue->clear_color[1],
			     queue->clear_color[2], queue->clear_color[3]);
		flags |= GL_COLOR_BUFFER_BIT;
	}

	if (queue->flags & DRAW_QUEUE_CLEAR_DEPTH) {
		glClearDepth(1.0);
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	queue->flags = 0;

	if (flags)
		glClear(flags);

	CHECK_GL;
	while (retry && draw_queue_try_flush(queue)) {
		retry = 0;

		for (i = 0; i < queue->draw_op_count; i++) {
			if (queue->draw_ops[i]->mesh->ebuf &&
			    queue->draw_ops[i]->mesh->vbuf)
				continue;

			retry = 1;

			draw_queue_add_mesh(queue, queue->draw_ops[i]->mesh);
		}
	}
}

/**
 * Set what color to use when we clear the buffer.
 *
 * r,g,b,a: The color to clear to.
 **/
void
draw_queue_set_clear_color(draw_queue_t *queue,
			   float r, float g, float b, float a)
{
	queue->clear_color[0] = r;
	queue->clear_color[1] = g;
	queue->clear_color[2] = b;
	queue->clear_color[3] = a;
}

/**
 * Clear the target buffer before the next flush.
 *
 * color: Clear the color buffer.
 * depth: Clear the depth buffer.
 **/
void
draw_queue_clear(draw_queue_t *queue, int color, int depth)
{
	if (color)
		queue->flags |= DRAW_QUEUE_CLEAR;

	if (depth)
		queue->flags |= DRAW_QUEUE_CLEAR_DEPTH;
}

/**
 * Set whether to clear the depth buffer before drawing.
 **/
void
draw_queue_set_clear_depth(draw_queue_t *queue, int flag)
{
	if (flag)
		queue->flags |= DRAW_QUEUE_CLEAR_DEPTH;
	else
		queue->flags &= ~DRAW_QUEUE_CLEAR_DEPTH;
}

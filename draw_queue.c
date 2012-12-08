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
	shader_uniform_type_t type;
	void *data;
};

/**
 * A draw operation. More or less corresponds to a single draw call.
 **/
struct draw_op {
	material_t *material;
	mesh_t *mesh;
	size_t pass;
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
	
	queue->flags = 0;
	queue->draw_ops = NULL;
	queue->draw_op_count = 0;
	queue->uniforms = NULL;
	queue->uniform_count = 0;

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
 * pass: The pass we are drawing.
 * transform: The transform to apply to the mesh.
 * camera: The camera we are drawing with.
 **/
static void
draw_queue_add_op(draw_queue_t *queue, object_t *object, size_t pass,
		  float transform[16], camera_t *camera)
{
	struct draw_op *op = xmalloc(sizeof(struct draw_op));

	queue->draw_ops = vec_expand(queue->draw_ops, queue->draw_op_count);

	op->material = object->material;
	op->pass = pass;
	op->mesh = object->mesh;
	op->uniforms = xcalloc(2, sizeof(struct uniform));

	op->uniforms[0].name = "transform";
	op->uniforms[0].type = SHADER_UNIFORM_MAT4;
	op->uniforms[0].data = xcalloc(16, sizeof(float));
	memcpy(op->uniforms[0].data, transform, 16 * sizeof(float));

	op->uniforms[1].name = "camera_transform";
	op->uniforms[1].type = SHADER_UNIFORM_MAT4;
	op->uniforms[1].data = xcalloc(16, sizeof(float));
	memcpy(op->uniforms[1].data, camera->normal_xfrm,
	       16 * sizeof(float));

	op->uniform_count = 2;

	draw_queue_add_mesh(queue, object->mesh);

	queue->draw_ops[queue->draw_op_count++] = op;
}

/**
 * Queue a draw operation for the given object. Draw for the given pass and
 * apply the given transform.
 **/
static void
draw_queue_draw_matrix(draw_queue_t *queue, object_t *object, size_t pass,
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
			draw_queue_add_op(queue, object, pass, transform,
					  camera);

		for (;(size_t)i >= object->child_count; i++) {
			i = object_cursor_up(&cursor);

			if (i < 0)
				return;

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

	object_cursor_release(&cursor);
	matrix_stack_release(&pt_stack);
}

/**
 * Queue a draw operation for the given object. Draw for the given pass from
 * the perspective of the given camera.
 **/
void
draw_queue_draw(draw_queue_t *queue, object_t *object, size_t pass,
		camera_t *camera)
{
	draw_queue_draw_matrix(queue, object, pass, camera->to_clip_xfrm,
			       camera);
}

/**
 * Execute a draw operation.
 **/
static int
draw_queue_exec_op(struct draw_op *op)
{
	size_t i;

	material_activate(op->material, op->pass);

	for (i = 0; i < op->uniform_count; i++)
		if (op->uniforms[i].type == SHADER_UNIFORM_MAT4)
			shader_set_uniform_mat(op->material->shaders[op->pass],
					       op->uniforms[i].name,
					       op->uniforms[i].data);

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
	struct draw_op *op;

	for (i = 0; i < queue->pool_count; i++)
		bufpool_end_generation(queue->pools[i]);

	i = 0;
	for (j = 0; j < queue->draw_op_count; j++) {
		queue->draw_ops[i] = queue->draw_ops[j];
		op = queue->draw_ops[i];

		if (draw_queue_exec_op(op)) {
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
	size_t i;

	if (queue->flags & DRAW_QUEUE_CLEAR) {
		glClearColor(0.5, 0.0, 0.5, 0.0);
		flags |= GL_COLOR_BUFFER_BIT;
	}

	if (queue->flags & DRAW_QUEUE_CLEAR_DEPTH) {
		glClearDepth(1.0);
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	if (flags)
		glClear(flags);

	while (draw_queue_try_flush(queue))
		for (i = 0; i < queue->draw_op_count; i++)
			draw_queue_add_mesh(queue, queue->draw_ops[i]->mesh);
}

/**
 * Set whether to clear the color buffer before drawing.
 *
 * flag: 1 to clear before drawing, 0 to not clear.
 * r,g,b,a: The color to clear to.
 **/
void
draw_queue_set_clear(draw_queue_t *queue, int flag, float r, float g, float b,
		     float a)
{
	if (! flag)
		queue->flags &= ~DRAW_QUEUE_CLEAR;
	
	queue->flags |= DRAW_QUEUE_CLEAR;
	queue->clear_color[0] = r;
	queue->clear_color[1] = g;
	queue->clear_color[2] = b;
	queue->clear_color[3] = a;
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

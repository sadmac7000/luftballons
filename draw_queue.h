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

#ifndef DRAW_QUEUE_H
#define DRAW_QUEUE_H

#include "bufpool.h"
#include "mesh.h"
#include "util.h"
#include "object.h"
#include "shader.h"
#include "camera.h"

/**
 * Flags for the draw queue.
 **/
#define DRAW_QUEUE_CLEAR 1
#define DRAW_QUEUE_CLEAR_DEPTH 2

/**
 * A list of draw operations to be performed.
 **/
typedef struct draw_queue {
	bufpool_t **pools;
	size_t pool_count;
	list_head_t draw_ops;
	unsigned int flags;
	float clear_color[4];
} draw_queue_t;

#ifdef __cplusplus
extern "C" {
#endif

draw_queue_t *draw_queue_create(void);
void draw_queue_draw(draw_queue_t *queue, object_t *object, shader_t *shader,
		     camera_t *camera);
void draw_queue_flush(draw_queue_t *queue);
void draw_queue_set_clear(draw_queue_t *queue, int flag, float r, float g,
			  float b, float a);
void draw_queue_set_clear_depth(draw_queue_t *queue, int flag);

#ifdef __cplusplus
}
#endif

#endif /* DRAW_QUEUE_H */

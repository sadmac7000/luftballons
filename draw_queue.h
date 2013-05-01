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
 * A list of draw operations to be performed.
 *
 * pools: bufpools to back meshes we are drawing
 * pool_count: Number of pools
 * draw_ops: Draw operations pending
 * draw_op_count: Number of draw operations
 **/
typedef struct draw_queue {
	bufpool_t **pools;
	size_t pool_count;
	struct draw_op **draw_ops;
	size_t draw_op_count;
} draw_queue_t;

#ifdef __cplusplus
extern "C" {
#endif

draw_queue_t *draw_queue_create(void);
void draw_queue_draw(draw_queue_t *queue, object_t *object, camera_t *camera);
void draw_queue_flush(draw_queue_t *queue);

#ifdef __cplusplus
}
#endif

#endif /* DRAW_QUEUE_H */

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

/**
 * A list of draw operations to be performed.
 **/
typedef struct draw_queue {
	bufpool_t **pools;
	size_t pool_count;
} draw_queue_t;

#ifdef __cplusplus
extern "C" {
#endif

draw_queue_t *draw_queue_create(void);
void draw_queue_add_mesh(draw_queue_t *queue, mesh_t *mesh);
void draw_queue_flush(draw_queue_t *queue);

#ifdef __cplusplus
}
#endif

#endif /* DRAW_QUEUE_H */

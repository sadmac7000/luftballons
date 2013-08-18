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

#ifndef BUFPOOL_H
#define BUFPOOL_H

#include "util.h"
#include "vbuf_fmt.h"
#include "mesh.h"

/**
 * A pool of buffer space to draw from to back an object.
 *
 * format: What vertex format these buffers are.
 * generations: LRU list of backed meshes.
 * generation_over: Whether the next new mesh should start a generation.
 **/
typedef struct bufpool {
	vbuf_fmt_t format;
	list_head_t generations;
	int generation_over;
} bufpool_t;

#ifdef __cplusplus
extern "C" {
#endif

bufpool_t *bufpool_create(vbuf_fmt_t format);
void bufpool_end_generation(bufpool_t *pool);
void bufpool_add_mesh(bufpool_t *pool, mesh_t *mesh);

#ifdef __cplusplus
}
#endif

#endif /* BUFPOOL_H */


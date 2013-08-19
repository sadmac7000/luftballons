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

#include "bufpool.h"
#include "vbuf.h"
#include "ebuf.h"

/**
 * Meshes in the same generation were all backed around the same time.
 **/
struct generation {
	list_head_t meshes;
};

/**
 * Add a new generation to this pool.
 **/
static void
bufpool_add_generation(bufpool_t *pool)
{
	size_t i;

	for (i = 0; i < pool->num_generations;) {
		if (list_empty(&pool->generations[i].meshes)) {
			pool->generations = vec_del(pool->generations,
						    pool->num_generations, i);
			pool->num_generations--;
		} else {
			i++;
		}
	}

	pool->generations = vec_expand(pool->generations, pool->num_generations);

	list_init(&pool->generations[pool->num_generations++].meshes);
}

/**
 * Create a new buffer pool. Every pool serves buffers of exactly one format.
 **/
bufpool_t *
bufpool_create(vbuf_fmt_t format)
{
	bufpool_t *ret = xmalloc(sizeof(bufpool_t));

	ret->format = format;
	ret->generation_over = 1;
	ret->generations = NULL;
	ret->num_generations = 0;

	return ret;
}

/**
 * Free up some space in this bufpool.
 **/
static int
bufpool_prune(bufpool_t *pool)
{
	struct generation *gen;
	mesh_t *mesh;

	if (pool->num_generations <= 1)
		return 0;

	gen = &pool->generations[--pool->num_generations];

	while (! list_empty(&gen->meshes)) {
		mesh = CONTAINER_OF(gen->meshes.next, mesh_t, generation_link);
		list_remove(&mesh->generation_link);
		mesh->generation = NULL;

		mesh_remove_from_vbuf(mesh);
		mesh_remove_from_ebuf(mesh);
	}

	pool->generations = vec_contract(pool->generations, pool->num_generations);

	return 1;
}

/**
 * Allocate vbufs and ebufs for all the extant meshes we have.
 **/
static void
bufpool_create_buffers(bufpool_t *pool)
{
	struct generation *gen;
	mesh_t *mesh;
	size_t vbuf_size = 0;
	size_t ebuf_size = 0;
	vbuf_t *vbuf = NULL;
	ebuf_t *ebuf = NULL;

	if (! pool->num_generations)
		return;

	gen = &pool->generations[pool->num_generations - 1];

	foreach(&gen->meshes, pos) {
		mesh = CONTAINER_OF(pos, mesh_t, generation_link);

		if (! mesh->vbuf)
			vbuf_size += mesh->verts;

		if (! mesh->ebuf)
			ebuf_size += mesh->elems;
	}

	do {
		vbuf = vbuf_create(vbuf_size, pool->format);
	} while (!vbuf && bufpool_prune(pool));

	do {
		ebuf = ebuf_create(ebuf_size);
	} while (!ebuf && bufpool_prune(pool));

	/* FIXME: Prune generations on failure at least. */

	foreach(&gen->meshes, pos) {
		mesh = CONTAINER_OF(pos, mesh_t, generation_link);

		if (vbuf && !mesh->vbuf)
			mesh_add_to_vbuf(mesh, vbuf);

		if (ebuf && !mesh->ebuf)
			mesh_add_to_ebuf(mesh, ebuf);
	}

	if (ebuf)
		ebuf_ungrab(ebuf);
	if (vbuf)
		vbuf_ungrab(vbuf);
}

/**
 * End a generation in this pool.
 **/
void
bufpool_end_generation(bufpool_t *pool)
{
	pool->generation_over = 1;
	bufpool_create_buffers(pool);
}

/**
 * Add a mesh to this buffer pool in this generation.
 **/
void
bufpool_add_mesh(bufpool_t *pool, mesh_t *mesh)
{
	struct generation *gen;

	if (pool->generation_over)
		bufpool_add_generation(pool);

	pool->generation_over = 0;
	gen = &pool->generations[pool->num_generations - 1];

	if (gen == mesh->generation)
		return;

	if (mesh->format != pool->format)
		errx(1, "Added mesh to wrong buffer pool");

	list_insert(&gen->meshes, &mesh->generation_link);

	mesh->generation = gen;
}

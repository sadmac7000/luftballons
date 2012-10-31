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

#include <err.h>

#include "bufpool.h"
#include "vbuf.h"
#include "ebuf.h"

/**
 * Meshes in the same generation were all backed around the same time.
 **/
struct generation {
	list_node_t link;
	list_head_t meshes;
};

/**
 * Add a new generation to this pool.
 **/
static void
bufpool_add_generation(bufpool_t *pool)
{
	struct generation *gen = xmalloc(sizeof(struct generation));

	list_init(&gen->link);
	list_init(&gen->meshes);
	list_insert(&pool->generations, &gen->link);
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

	list_init(&ret->generations);

	return ret;
}

/**
 * Free up some space in this bufpool.
 **/
static int
bufpool_prune(bufpool_t *pool)
{
	struct generation *gen = (struct generation *)pool->generations.prev;
	mesh_t *mesh;

	/* 0 or 1 generations left */
	if (pool->generations.prev == pool->generations.next)
		return 0;

	list_remove(&gen->link);

	while (! list_empty(&gen->meshes)) {
		mesh = CONTAINER_OF(gen->meshes.next, mesh_t, generation_link);
		list_remove(&mesh->generation_link);
		mesh->generation = NULL;

		mesh_remove_from_vbuf(mesh);
		mesh_remove_from_ebuf(mesh);
	}

	free(gen);
	return 1;
}

/**
 * Allocate vbufs and ebufs for all the extant meshes we have.
 **/
static void
bufpool_create_buffers(bufpool_t *pool)
{
	struct generation *gen = (struct generation *)pool->generations.next;
	mesh_t *mesh;
	size_t vbuf_size = 0;
	size_t ebuf_size = 0;
	vbuf_t *vbuf;
	ebuf_t *ebuf;

	foreach(&gen->meshes) {
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

	foreach(&gen->meshes) {
		mesh = CONTAINER_OF(pos, mesh_t, generation_link);

		if (vbuf && !mesh->vbuf)
			mesh_add_to_vbuf(mesh, vbuf);

		if (ebuf && !mesh->ebuf)
			mesh_add_to_ebuf(mesh, ebuf);
	}
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
 * Notify that a generation has been lost.
 *
 * Note that this is not exposed in the header file. It is prototyped for
 * mesh.c alone, which cannot include the header file due to circular
 * dependency.
 **/
void
bufpool_notify_generation(struct generation *gen)
{
	if (! list_empty(&gen->meshes))
		return;

	list_remove(&gen->link);
	free(gen);
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
	gen = (struct generation *)pool->generations.next;

	if (gen == mesh->generation)
		return;

	if (mesh->format != pool->format)
		errx(1, "Added mesh to wrong buffer pool");

	list_insert(&gen->meshes, &mesh->generation_link);

	if (mesh->generation)
		bufpool_notify_generation(mesh->generation);

	mesh->generation = gen;
}

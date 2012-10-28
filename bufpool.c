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

	list_init(&ret->generations);

	bufpool_add_generation(ret);
	return ret;
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

	vbuf = vbuf_create(vbuf_size, pool->format);
	ebuf = ebuf_create(ebuf_size);

	/* FIXME: Prune generations on failure at least. */

	if (vbuf_size && !vbuf)
		errx(1, "Could not allocate vbuf");

	if (ebuf_size && !ebuf)
		errx(1, "Could not allocate ebuf");

	foreach(&gen->meshes) {
		mesh = CONTAINER_OF(pos, mesh_t, generation_link);

		if (! mesh->vbuf)
			mesh_add_to_vbuf(mesh, vbuf);

		if (! mesh->ebuf)
			mesh_add_to_ebuf(mesh, ebuf);
	}
}

/**
 * End a generation in this pool.
 **/
void
bufpool_end_generation(bufpool_t *pool)
{
	bufpool_create_buffers(pool);
	bufpool_add_generation(pool);
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

	/* This test is designed to keep the only generation from getting
	 * reaped, since the rest of this file tends to work on the assumption
	 * that there is always one generation.
	 *
	 * It will not necessarily prevent the newest generation from getting
	 * reaped if that becomes empty before older generations. That's a
	 * weird case though, and the effect is just a blip in our age
	 * accounting.
	 */
	if (gen->meshes.next == gen->meshes.prev)
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
	struct generation *gen = (struct generation *)pool->generations.next;

	if (gen == mesh->generation)
		return;

	if (mesh->format != pool->format)
		errx(1, "Added mesh to wrong buffer pool");

	list_insert(&gen->meshes, &mesh->generation_link);

	if (mesh->generation)
		bufpool_notify_generation(mesh->generation);

	mesh->generation = gen;
}

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

#include "drawlist.h"
#include "shader.h"
#include "camera.h"

/**
 * Create a new drawlist.
 **/
drawlist_t *
drawlist_create(void)
{
	drawlist_t *ret = xmalloc(sizeof(drawlist_t));

	list_init(&ret->vbufs);
	list_init(&ret->ebufs);
	list_init(&ret->objects);

	ret->object_count = 0;

	return ret;
}

/**
 * Remove an object from the drawlist.
 **/
void
drawlist_remove_object(drawlist_t *drawlist, object_t *object)
{
	list_remove(&object->drawlist_link);
	drawlist->object_count--;
}

/**
 * Add an object to the drawlist.
 **/
void
drawlist_add_object(drawlist_t *drawlist, object_t *object)
{
	if (! list_empty(&object->drawlist_link))
		errx(1, "Cannot add object to multiple drawlists");

	list_insert(&drawlist->objects, &object->drawlist_link);
	drawlist->object_count++;
}

/**
 * Swap two mesh pointer values;
 **/
static void
mesh_swap(mesh_t **a, mesh_t **b)
{
	mesh_t *tmp = *a;
	*a = *b;
	*b = tmp;
}

/**
 * Sort an array of pointers to mesh based on pointer offset.
 **/
static void
sort_meshes_pointer(mesh_t **meshes, size_t count)
{
	size_t pivot;
	size_t i;

	if (count <= 1)
		return;

	if (count == 2) {
		if (meshes[0] > meshes[1])
			mesh_swap(&meshes[0], &meshes[1]);

		return;
	}

	pivot = count / 2;
	mesh_swap(&meshes[0], &meshes[pivot]);
	pivot = 0;

	for (i = 1; i < count; i++) {
		if (meshes[i] >= meshes[pivot])
			continue;

		mesh_swap(&meshes[i], &meshes[pivot + 1]);
		mesh_swap(&meshes[pivot], &meshes[pivot + 1]);
		pivot++;
	}

	sort_meshes_pointer(meshes, pivot);
	sort_meshes_pointer(meshes + pivot + 1, count - pivot - 1);
}

/**
 * Get all of the meshes the objects we are about to draw refer to.
 **/
static mesh_t **
drawlist_get_unique_meshes(drawlist_t *drawlist, size_t *len)
{
	mesh_t **meshes = xcalloc(drawlist->object_count, sizeof(mesh_t *));
	object_t *object;
	size_t count = 0;
	size_t i;
	size_t j;

	foreach(&drawlist->objects) {
		object = CONTAINER_OF(pos, object_t, drawlist_link);

		if (object->mesh)
			meshes[count++] = object->mesh;
	}

	if (! count) {
		free(meshes);
		*len = 0;
		return NULL;
	}

	sort_meshes_pointer(meshes, count);

	for (i = 0, j = 1; j < count; j++)
		if (meshes[j] != meshes[i])
			meshes[++i] = meshes[j];

	*len = i + 1;
	return meshes;
}

/**
 * Draw all objects on the draw list.
 **/
void
drawlist_draw(drawlist_t *drawlist, shader_t *shader, camera_t *camera)
{
	object_t *object;
	mesh_t **meshes;
	mesh_t **unbuffered_meshes;
	vbuf_t *vbuf;
	ebuf_t *ebuf;
	size_t count;
	size_t size;
	size_t i;
	size_t j;

	meshes = drawlist_get_unique_meshes(drawlist, &count);

	unbuffered_meshes = xcalloc(count, sizeof(mesh_t *));

	for (i = 0, j = 0; i < count; i++)
		if (! meshes[i]->vbuf)
			unbuffered_meshes[j++] = meshes[i];

	if (j) {
		size = 0;
		for (i = 0; i < j; i++)
			size += unbuffered_meshes[i]->verts;

		vbuf = vbuf_create(size, unbuffered_meshes[0]->format);
		list_insert(&drawlist->vbufs, &vbuf->drawlist_link);

		for (i = 0; i < j; i++)
			mesh_add_to_vbuf(unbuffered_meshes[i], vbuf);
	}

	for (i = 0, j = 0; i < count; i++)
		if (! meshes[i]->ebuf)
			unbuffered_meshes[j++] = meshes[i];

	if (j) {
		size = 0;
		for (i = 0; i < j; i++)
			size += unbuffered_meshes[i]->elems;

		ebuf = ebuf_create(size);
		list_insert(&drawlist->ebufs, &ebuf->drawlist_link);

		for (i = 0; i < j; i++)
			mesh_add_to_ebuf(unbuffered_meshes[i], ebuf);
	}

	free(unbuffered_meshes);
	free(meshes);

	foreach(&drawlist->objects) {
		object = CONTAINER_OF(pos, object_t, drawlist_link);

		if (! object->parent)
			object_draw(object, shader, camera);
	}
}

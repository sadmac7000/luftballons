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

#include <stdlib.h>
#include <string.h>

#include "mesh.h"
#include "util.h"

/**
 * Create a new mesh object.
 **/
mesh_t *
mesh_create(shader_t *shader, size_t verts, const float *vert_data,
	    size_t elems, const uint16_t *elem_data, size_t segments,
	    vbuf_fmt_t *segment_descriptors, GLenum type)
{
	mesh_t *ret = xmalloc(sizeof(mesh_t));
	size_t data_size = verts;
	size_t i;

	for (i = 0; i < segments; i++)
		data_size *= vbuf_segment_size(&segment_descriptors[i]);

	ret->vert_data = xmalloc(data_size);
	memcpy(ret->vert_data, vert_data, data_size);
	ret->verts = verts;

	ret->elem_data = xmalloc(data_size);
	memcpy(ret->elem_data, elem_data, 2 * elems);
	ret->elems = elems;

	ret->segments = segments;
	ret->segment_descriptors = xcalloc(segments, sizeof(vbuf_fmt_t));

	ret->shader = shader;
	ret->type = type;

	ret->vbuf = NULL;
	ret->vbuf_pos = 0;

	ret->ebuf = NULL;
	ret->ebuf_pos = 0;

	ret->refcount = 0;

	return ret;
}

/**
 * Get a reference to a mesh.
 **/
void
mesh_grab(mesh_t *mesh)
{
	mesh->refcount++;
}

/**
 * Unget a reference to a mesh.
 **/
void
mesh_ungrab(mesh_t *mesh)
{
	if (! mesh->refcount)
		errx(1, "Mesh refcount went negative");

	if (--mesh->refcount)
		return;

	mesh_destroy(mesh);
}

/**
 * Destroy and free a mesh object.
 **/
void
mesh_destroy(mesh_t *mesh)
{
	mesh_remove_from_vbuf(mesh);
	mesh_remove_from_ebuf(mesh);

	free(mesh->vert_data);
	free(mesh);
}

/**
 * Attach a mesh to an element buffer.
 *
 * Returns: 0 on success, -1 on no space.
 **/
int
mesh_add_to_ebuf(mesh_t *mesh, ebuf_t *buffer)
{
	ssize_t offset = ebuf_locate_free_space(buffer, mesh->elems);

	if (offset < 0)
		return -1;

	mesh_remove_from_ebuf(mesh);

	ebuf_alloc_region(buffer, offset, mesh->elems);

	ebuf_activate(buffer);

	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(uint16_t),
			mesh->elems * sizeof(uint16_t), mesh->elem_data);

	mesh->ebuf = buffer;
	mesh->ebuf_pos = offset;
	ebuf_grab(buffer);

	return 0;
}

/**
 * Attach a mesh to a vertex buffer.
 *
 * Returns: 0 on success, -1 on no space.
 **/
int
mesh_add_to_vbuf(mesh_t *mesh, vbuf_t *buffer)
{
	ssize_t offset = vbuf_locate_free_space(buffer, mesh->verts);
	size_t base = 0;
	size_t local_base = 0;
	size_t i;
	vbuf_fmt_t *seg;

	if (offset < 0)
		return -1;

	mesh_remove_from_vbuf(mesh);

	vbuf_alloc_region(buffer, offset, mesh->verts);

	mesh->vbuf = buffer;
	mesh->vbuf_pos = offset;

	vbuf_activate(buffer);

	for (i = 0; i < buffer->segments; i++) {
		seg = &buffer->segment_descriptors[i];

		glBufferSubData(GL_ARRAY_BUFFER,
				base + offset * vbuf_segment_size(seg),
				mesh->verts * vbuf_segment_size(seg),
				mesh->vert_data + local_base);

		base += vbuf_segment_size(seg) * buffer->vert_count;
		local_base += vbuf_segment_size(seg) * mesh->verts;
	}

	vbuf_grab(buffer);

	return 0;
}

/**
 * Remove a mesh from its host element buffer.
 **/
void
mesh_remove_from_ebuf(mesh_t *mesh)
{
	if (! mesh->ebuf)
		return;

	ebuf_drop_data(mesh->ebuf, mesh->ebuf_pos, mesh->elems);
	ebuf_ungrab(mesh->ebuf);
	mesh->ebuf = NULL;
}

/**
 * Remove a mesh from its host vertex buffer.
 **/
void
mesh_remove_from_vbuf(mesh_t *mesh)
{
	if (! mesh->vbuf)
		return;

	vbuf_drop_data(mesh->vbuf, mesh->vbuf_pos, mesh->verts);
	vbuf_ungrab(mesh->vbuf);
	mesh->vbuf = NULL;
}

/**
 * Draw a mesh.
 **/
void
mesh_draw(mesh_t *mesh)
{
	if (! mesh->vbuf)
		errx(1, "Drawing mesh without vertex buffer");

	if (! mesh->ebuf)
		errx(1, "Drawing mesh without element buffer");

	shader_activate(mesh->shader, mesh->vbuf);
	ebuf_activate(mesh->ebuf);

	/*glDrawArrays(mesh->type, mesh->vbuf_pos, mesh->verts);*/
	glDrawElementsBaseVertex(mesh->type, mesh->elems, GL_UNSIGNED_SHORT,
				 (void *)mesh->ebuf_pos, mesh->vbuf_pos);
}

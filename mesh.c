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
	    size_t segments, vbuf_fmt_t *segment_descriptors, GLenum type)
{
	mesh_t *ret = xmalloc(sizeof(mesh_t));
	size_t data_size = verts;
	size_t i;

	for (i = 0; i < segments; i++)
		data_size *= segment_descriptors[i].size;

	ret->vert_data = xmalloc(data_size);
	memcpy(ret->vert_data, vert_data, data_size);
	ret->segments = segments;
	ret->segment_descriptors = xcalloc(segments, sizeof(vbuf_fmt_t));
	ret->shader = shader;
	ret->verts = verts;
	ret->buffer = NULL;
	ret->buffer_pos = 0;
	ret->type = type;

	return ret;
}

/**
 * Destroy and free a mesh object.
 **/
void
mesh_destroy(mesh_t *mesh)
{
	mesh_remove_from_buffer(mesh);

	free(mesh->vert_data);
	free(mesh);
}

/**
 * Attach a mesh to a buffer.
 *
 * Returns: 0 on success, -1 on no space.
 **/
int
mesh_add_to_buffer(mesh_t *mesh, vbuf_t *buffer)
{
	ssize_t offset = vbuf_locate_free_space(buffer, mesh->verts);
	size_t base = 0;
	size_t local_base = 0;
	size_t i;
	vbuf_fmt_t *seg;

	if (offset < 0)
		return -1;

	mesh_remove_from_buffer(mesh);

	vbuf_alloc_region(buffer, offset, mesh->verts);

	mesh->buffer = buffer;
	mesh->buffer_pos = offset;

	vbuf_bind(buffer);

	for (i = 0; i < buffer->segments; i++) {
		seg = &buffer->segment_descriptors[i];

		glBufferSubData(GL_ARRAY_BUFFER, base + offset * seg->size,
				mesh->verts * seg->size,
				mesh->vert_data + local_base);

		base += seg->size * buffer->vert_count;
		local_base += seg->size * mesh->verts;
	}

	vbuf_grab(buffer);

	return 0;
}

/**
 * Remove a mesh from its host buffer.
 **/
void
mesh_remove_from_buffer(mesh_t *mesh)
{
	if (! mesh->buffer)
		return;

	vbuf_drop_data(mesh->buffer, mesh->buffer_pos, mesh->verts);
	vbuf_ungrab(mesh->buffer);
	mesh->buffer = NULL;
}

/**
 * Draw a mesh.
 **/
void
mesh_draw(mesh_t *mesh)
{
	if (! mesh->buffer)
		errx(1, "Drawing mesh without buffer");

	shader_activate(mesh->shader, mesh->buffer);

	glDrawArrays(mesh->type, mesh->buffer_pos, mesh->verts);
}

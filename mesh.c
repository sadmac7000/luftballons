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
	    size_t segments, buf_vdata_t *segment_descriptors)
{
	mesh_t *ret = xmalloc(sizeof(mesh_t));
	size_t data_size = verts;
	size_t i;

	for (i = 0; i < segments; i++)
		data_size *= segment_descriptors[i].size;

	ret->vert_data = xmalloc(data_size);
	memcpy(ret->vert_data, vert_data, data_size);
	ret->segments = segments;
	ret->segment_descriptors = xcalloc(segments, sizeof(buf_vdata_t));
	ret->shader = shader;
	ret->verts = verts;
	ret->buffer = NULL;
	ret->buffer_pos = 0;

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
mesh_add_to_buffer(mesh_t *mesh, buffer_t *buffer)
{
	ssize_t offset = buffer_locate_free_space(buffer, mesh->verts);

	if (offset < 0)
		return -1;

	mesh_remove_from_buffer(mesh);

	buffer_add_data(buffer, offset, mesh->verts, mesh->vert_data);
	mesh->buffer = buffer;
	mesh->buffer_pos = offset;
	buffer_grab(buffer);

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

	buffer_drop_data(mesh->buffer, mesh->buffer_pos, mesh->verts);
	buffer_ungrab(mesh->buffer);
	mesh->buffer = NULL;
}

int
callback(const char *name, GLuint value, void *data)
{
	mesh_t *mesh = data;

	if (! strcmp(name, "position"))
		glVertexAttribPointer(value, 4, GL_FLOAT, GL_FALSE, 0,
				      (void *)mesh->buffer_pos);
	else if (! strcmp(name, "colorin"))
		glVertexAttribPointer(value, 4, GL_FLOAT, GL_FALSE, 0,
				      (void *)(mesh->buffer_pos +
				       (12 * sizeof(float))));
	else
		return 0;

	return 1;
}

/**
 * Draw a mesh.
 **/
void
mesh_draw(mesh_t *mesh)
{
	if (! mesh->buffer)
		errx(1, "Drawing mesh without buffer");

	buffer_bind(mesh->buffer);
	shader_activate(mesh->shader);

	shader_set_vertex_attrs(mesh->shader, callback, mesh);

	glDrawArrays(GL_TRIANGLES, 0, mesh->verts);
}

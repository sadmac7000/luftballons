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
mesh_create(shader_t *shader, size_t verts, size_t vert_colors,
	    const float *vert_data)
{
	mesh_t *ret = xmalloc(sizeof(mesh_t));

	ret->vert_data = xcalloc(verts + vert_colors, sizeof(float));
	memcpy(ret->vert_data, vert_data, (verts + vert_colors) * sizeof(float));
	ret->shader = shader;
	ret->verts = verts;
	ret->vert_colors = vert_colors;
	ret->buffer = NULL;
	ret->buffer_pos = 0;

	return ret;
}

/**
 * Find the total size of the mesh's array data.
 **/
size_t
mesh_data_size(mesh_t *mesh)
{
	return mesh->verts + mesh->vert_colors;
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
	size_t size = mesh_data_size(mesh);
	ssize_t offset = buffer_locate_free_space(buffer, size);

	if (offset < 0)
		return -1;

	mesh_remove_from_buffer(mesh);

	buffer_add_data(buffer, offset, size, mesh->vert_data);
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
	size_t size = mesh_data_size(mesh);

	if (! mesh->buffer)
		return;

	buffer_drop_data(mesh->buffer, mesh->buffer_pos, size);
	buffer_ungrab(mesh->buffer);
	mesh->buffer = NULL;
}

/**
 * Draw a mesh.
 **/
void
mesh_draw(mesh_t *mesh)
{
	GLint vert_pos_loc;
	GLint color_pos_loc;

	if (! mesh->buffer)
		errx(1, "Drawing mesh without buffer");

	buffer_bind(mesh->buffer);
	shader_activate(mesh->shader);

	vert_pos_loc = glGetAttribLocation(mesh->shader->gl_handle, "position");
	color_pos_loc = glGetAttribLocation(mesh->shader->gl_handle, "colorin");

	glEnableVertexAttribArray(vert_pos_loc);
	glEnableVertexAttribArray(color_pos_loc);
	glVertexAttribPointer(vert_pos_loc, 4, GL_FLOAT, GL_FALSE, 0,
			      (void *)mesh->buffer_pos);
	glVertexAttribPointer(color_pos_loc, 4, GL_FLOAT, GL_FALSE, 0,
			      (void *)(mesh->buffer_pos +
				       (12 * sizeof(float))));
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

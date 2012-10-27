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

#ifndef MESH_H
#define MESH_H

#include <GL/glut.h>

#include "vbuf.h"
#include "vbuf_fmt.h"
#include "ebuf.h"

/**
 * A collection of vertex data ready to be passed to a draw call.
 *
 * vert_data: Copy of the vertex data we will pass in to the shader.
 * verts: Number of vertices.
 * type: Type of GL primitives this vertex data represents.
 * elem_data: Element buffer for this vertex data.
 * elems: Number of elements in this vertex data.
 * format: Format of the vertex data.
 * vbuf: Vertex buffer we are currently copied in to.
 * vbuf_pos: Where in the vertex buffer we've been loaded.
 * ebuf: ELement buffer we are currently copied in to.
 * ebuf_pos: Where in the element buffer we've been loaded.
 * refcount: Refcount for tracking and freeing this object.
 * generation_link: Link into a bufpool generation.
 **/
typedef struct bufpool bufpool_t;
typedef struct mesh {
	void *vert_data;
	size_t verts;
	GLenum type;

	char *elem_data;
	size_t elems;

	vbuf_fmt_t format;

	list_node_t generation_link;

	vbuf_t *vbuf;
	size_t vbuf_pos;

	ebuf_t *ebuf;
	size_t ebuf_pos;

	size_t refcount;
} mesh_t;

#ifdef __cplusplus
extern "C" {
#endif

mesh_t *mesh_create(size_t verts, const void *vert_data, size_t elems,
		    const uint16_t *elem_data, vbuf_fmt_t format, GLenum type);
size_t mesh_data_size(mesh_t *mesh);
void mesh_destroy(mesh_t *mesh);
int mesh_add_to_vbuf(mesh_t *mesh, vbuf_t *buffer);
int mesh_add_to_ebuf(mesh_t *mesh, ebuf_t *buffer);
void mesh_remove_from_vbuf(mesh_t *mesh);
void mesh_remove_from_ebuf(mesh_t *mesh);
void mesh_draw(mesh_t *mesh);
void mesh_grab(mesh_t *mesh);
void mesh_ungrab(mesh_t *mesh);

#ifdef __cplusplus
}
#endif

#endif /* MESH_H */


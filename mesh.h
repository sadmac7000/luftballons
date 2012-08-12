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

#include "buffer.h"

typedef struct mesh {
	float *vert_data;
	GLint shader_prog;
	size_t verts;
	size_t vert_colors;

	buffer_t *buffer;
	size_t buffer_pos;
} mesh_t;

#ifdef __cplusplus
extern "C" {
#endif

mesh_t *mesh_create(GLint shader_prog, size_t verts, size_t vert_colors,
		    const float *vert_data);
size_t mesh_data_size(mesh_t *mesh);
void mesh_destroy(mesh_t *mesh);
int mesh_add_to_buffer(mesh_t *mesh, buffer_t *buffer);
void mesh_remove_from_buffer(mesh_t *mesh);
void mesh_draw(mesh_t *mesh);

#ifdef __cplusplus
}
#endif

#endif /* MESH_H */


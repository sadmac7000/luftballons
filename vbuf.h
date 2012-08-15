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

#ifndef VBUF_H
#define VBUF_H

#include <GL/glut.h>

#include "interval.h"

/**
 * A type of vertex data that will be stored for each vertex. We use arrays of
 * these to define the complete vertex data.
 **/
typedef struct vbuf_fmt {
	char name[32];
	size_t size;
} vbuf_fmt_t;

/**
 * Wrapper around an OpenGL buffer object for vertex data buffers.
 *
 * gl_handle: OpenGL handle for the buffer.
 * segments: Number of segments in the buffer.
 * segment_descriptors: Type of data in each segment.
 * vert_size: size of the data for a vertex in bytes.
 * vert_count: total verts in this buffer.
 * refcount: Reference counting.
 * free: Free space tracking.
 **/
typedef struct vbuf {
	GLuint gl_handle;

	size_t segments;
	vbuf_fmt_t *segment_descriptors;
	size_t vert_size;
	size_t vert_count;

	unsigned int refcount;

	intervals_t free;
} vbuf_t;

#ifdef __cplusplus
extern "C" {
#endif

vbuf_t *vbuf_create(size_t size, GLenum usage, size_t segments,
		    vbuf_fmt_t *segment_descriptors);
void vbuf_grab(vbuf_t *buffer);
void vbuf_ungrab(vbuf_t *buffer);
void vbuf_drop_data(vbuf_t *buffer, size_t offset, size_t size);
void vbuf_bind(vbuf_t *buffer);
void vbuf_alloc_region(vbuf_t *buffer, size_t offset, size_t size);
ssize_t vbuf_locate_free_space(vbuf_t *buffer, size_t size);
void vbuf_setup_vertex_attribute(vbuf_t *buffer,
				   const char *name, GLint handle);

#ifdef __cplusplus
}
#endif

#endif /* VBUF_H */


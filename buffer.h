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

#ifndef BUFFER_H
#define BUFFER_H

#include <GL/glut.h>

/**
 * A type of vertex data that will be stored for each vertex.
 **/
typedef struct buf_vdata {
	char name[32];
	size_t size;
} buf_vdata_t;

/**
 * A region in a buffer.
 *
 * start: Start offset.
 * size: Byte length.
 **/
typedef struct buf_region {
	size_t start;
	size_t size;
} buf_region_t;

/**
 * Wrapper around an OpenGL buffer object.
 *
 * gl_handle: OpenGL handle for the buffer.
 * segments: Number of segments in the buffer.
 * segment_descriptors: Type of data in each segment.
 * vert_size: size of the data for a vertex in bytes.
 * vert_count: total verts in this buffer.
 * refcount: Reference counting.
 * regions_sz: Array of free regions by size.
 * regions_off: Array of free regions by position.
 * region_count: Number of free regions.
 **/
typedef struct buffer {
	GLuint gl_handle;

	size_t segments;
	buf_vdata_t *segment_descriptors;
	size_t vert_size;
	size_t vert_count;

	unsigned int refcount;

	buf_region_t **regions_sz;
	buf_region_t **regions_off;
	size_t region_count;
} buffer_t;

#ifdef __cplusplus
extern "C" {
#endif

buffer_t *buffer_create(size_t size, GLenum usage, size_t segments,
			buf_vdata_t *segment_descriptors);
void buffer_grab(buffer_t *buffer);
void buffer_ungrab(buffer_t *buffer);
void buffer_drop_data(buffer_t *buffer, size_t offset, size_t size);
void buffer_bind(buffer_t *buffer);
void buffer_alloc_region(buffer_t *buffer, size_t offset, size_t size);
ssize_t buffer_locate_free_space(buffer_t *buffer, size_t size);
void buffer_setup_vertex_attribute(buffer_t *buffer,
				   const char *name, GLint handle);

#ifdef __cplusplus
}
#endif

#endif /* BUFFER_H */


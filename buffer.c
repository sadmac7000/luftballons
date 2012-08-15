/**
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version/
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
#include <err.h>

#include "buffer.h"
#include "util.h"

buffer_t *current_array_buffer = NULL;

/**
 * Create a new buffer object.
 *
 * size: Vertices the buffer should accomodate.
 * usage: Use for the buffer.
 * segments: Number of segments.
 * segment_descriptors: Type of each segment.
 **/
buffer_t *
buffer_create(size_t size, GLenum usage, size_t segments,
	      buf_vdata_t *segment_descriptors)
{
	buffer_t *ret;
	GLuint handle;
	GLenum error;
	size_t segments_sz = segments * sizeof(buf_vdata_t);
	size_t i;
	GLsizeiptr byte_size = 0;

	for (i = 0; i < segments; i++)
		byte_size += segment_descriptors[i].size;

	glGenBuffers(1, &handle);
	glBindBuffer(GL_ARRAY_BUFFER, handle);
	glBufferData(GL_ARRAY_BUFFER, byte_size * size, NULL, usage);
	error = glGetError();

	if (current_array_buffer)
		buffer_bind(current_array_buffer);

	if (error != GL_NO_ERROR) {
		if (error == GL_OUT_OF_MEMORY)
			return NULL;

		errx(1, "Unexpected OpenGL error allocating buffer");
	}

	ret = xmalloc(sizeof(buffer_t));
	ret->vert_size = byte_size;
	ret->vert_count = size;
	ret->gl_handle = handle;
	ret->segments = segments;
	ret->segment_descriptors = xmalloc(segments_sz);
	memcpy(ret->segment_descriptors, segment_descriptors, segments_sz);
	ret->refcount = 1;

	intervals_init(&ret->free);
	interval_set(&ret->free, 0, size);

	return ret;
}

/**
 * Setup the vertex attribute named for the attribute handle given.
 **/
void
buffer_setup_vertex_attribute(buffer_t *buffer, const char *name, GLint handle)
{
	size_t i;
	buf_vdata_t *seg;
	size_t position = 0;

	for (i = 0; i < buffer->segments; i++) {
		seg = &buffer->segment_descriptors[i];

		if (! strcmp(seg->name, name))
			break;

		position += seg->size * buffer->vert_count;
	}

	if (i == buffer->segments) {
		glDisableVertexAttribArray(i);
		return;
	}

	buffer_bind(buffer);
	glEnableVertexAttribArray(i);
	glVertexAttribPointer(handle, 4, GL_FLOAT, GL_FALSE, 0,
			      (void *)position);
}

/**
 * Increase a buffer's refcount.
 **/
void
buffer_grab(buffer_t *buffer)
{
	buffer->refcount++;
}

/**
 * Decrease a buffer's refcount. If the count becomes zero, free it.
 **/
void
buffer_ungrab(buffer_t *buffer)
{
	if (--buffer->refcount)
		return;

	if (current_array_buffer == buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		current_array_buffer = NULL;
	}

	glDeleteBuffers(1, &buffer->gl_handle);

	intervals_release(&buffer->free);
	free(buffer);
}

/**
 * Create a new free region and add it to a buffer's lists.
 **/
void
buffer_drop_data(buffer_t *buffer, size_t offset, size_t size)
{
	interval_set(&buffer->free, offset, size);
}

/**
 * Allocate a region of a buffer.
 **/
void
buffer_alloc_region(buffer_t *buffer, size_t offset, size_t size)
{
	interval_unset(&buffer->free, offset, size);
}

/**
 * Bind a vertex buffer.
 **/
void
buffer_bind(buffer_t *buffer)
{
	current_array_buffer = buffer;
	glBindBuffer(GL_ARRAY_BUFFER, buffer->gl_handle);
}

/**
 * Locate some free space in a buffer.
 *
 * buffer: Buffer to search.
 * size: Size of free space to find.
 *
 * Returns: Offset to space or -1 if not enough space found.
 **/
ssize_t
buffer_locate_free_space(buffer_t *buffer, size_t size)
{
	return interval_find(&buffer->free, size);
}

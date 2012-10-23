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

#include "vbuf.h"
#include "util.h"

/* We keep this out of shader.h to prevent circular dependency. */
void shader_set_vertex_attrs();

vbuf_t *current_vbuf = NULL;

static size_t type_sizes[] = {
	[GL_BYTE] = 1,
	[GL_UNSIGNED_BYTE] = 1,
	[GL_SHORT] = sizeof(short),
	[GL_UNSIGNED_SHORT] = sizeof(short),
	[GL_INT] = sizeof(int),
	[GL_UNSIGNED_INT] = sizeof(int),

	/* Sort of a guess */
	[GL_HALF_FLOAT] = sizeof(float) / 2,

	[GL_FLOAT] = sizeof(float),
	[GL_DOUBLE] = sizeof(double),

	/* http://www.opengl.org/registry/specs/OES/OES_fixed_point.txt */
	[GL_FIXED] = 4,

	[GL_INT_2_10_10_10_REV] = sizeof(int),
	[GL_UNSIGNED_INT_2_10_10_10_REV] = sizeof(int),
};

static size_t type_sizes_max = sizeof(type_sizes) / sizeof(size_t) - 1;

/**
 * Find the expected size per element for a given segment in a vertex buffer.
 **/
size_t
vbuf_segment_size(vbuf_fmt_t *seg)
{
	size_t ret = 0;

	if (seg->type <= type_sizes_max)
		ret = type_sizes[seg->type];

	if (ret)
		return ret * seg->size;

	errx(1, "Tried to get size of invalid GL type");
}

/**
 * Create a new buffer object.
 *
 * size: Vertices the buffer should accomodate.
 * segments: Number of segments.
 * segment_descriptors: Type of each segment.
 **/
vbuf_t *
vbuf_create(size_t size, size_t segments, vbuf_fmt_t *segment_descriptors)
{
	vbuf_t *ret;
	GLuint handle;
	GLenum error;
	size_t segments_sz = segments * sizeof(vbuf_fmt_t);
	size_t i;
	GLsizeiptr byte_size = 0;

	for (i = 0; i < segments; i++)
		byte_size += vbuf_segment_size(&segment_descriptors[i]);

	glGenBuffers(1, &handle);
	glBindBuffer(GL_ARRAY_BUFFER, handle);
	glBufferData(GL_ARRAY_BUFFER, byte_size * size, NULL, GL_STATIC_DRAW);
	error = glGetError();

	if (current_vbuf)
		vbuf_activate(current_vbuf);

	if (error != GL_NO_ERROR) {
		if (error == GL_OUT_OF_MEMORY)
			return NULL;

		errx(1, "Unexpected OpenGL error allocating buffer");
	}

	ret = xmalloc(sizeof(vbuf_t));
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
vbuf_setup_vertex_attribute(const char *name, GLint handle)
{
	size_t i;
	vbuf_fmt_t *seg;
	size_t position = 0;

	if (! current_vbuf)
		return;

	for (i = 0; i < current_vbuf->segments; i++) {
		seg = &current_vbuf->segment_descriptors[i];

		if (! strcmp(seg->name, name))
			break;

		position += vbuf_segment_size(seg) * current_vbuf->vert_count;
	}

	if (i == current_vbuf->segments) {
		glDisableVertexAttribArray(i);
		return;
	}

	glEnableVertexAttribArray(i);
	glVertexAttribPointer(handle, seg->size, seg->type, GL_FALSE, 0,
			      (void *)position);
}

/**
 * Increase a buffer's refcount.
 **/
void
vbuf_grab(vbuf_t *buffer)
{
	buffer->refcount++;
}

/**
 * Decrease a buffer's refcount. If the count becomes zero, free it.
 **/
void
vbuf_ungrab(vbuf_t *buffer)
{
	if (--buffer->refcount)
		return;

	if (current_vbuf == buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		current_vbuf = NULL;
	}

	glDeleteBuffers(1, &buffer->gl_handle);

	intervals_release(&buffer->free);
	free(buffer);
}

/**
 * Create a new free region and add it to a buffer's lists.
 **/
void
vbuf_drop_data(vbuf_t *buffer, size_t offset, size_t size)
{
	interval_set(&buffer->free, offset, size);
}

/**
 * Allocate a region of a buffer.
 **/
void
vbuf_alloc_region(vbuf_t *buffer, size_t offset, size_t size)
{
	interval_unset(&buffer->free, offset, size);
}

/**
 * Bind a vertex buffer.
 **/
void
vbuf_activate(vbuf_t *buffer)
{
	if (current_vbuf == buffer)
		return;

	current_vbuf = buffer;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->gl_handle);
	shader_set_vertex_attrs();
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
vbuf_locate_free_space(vbuf_t *buffer, size_t size)
{
	return interval_find(&buffer->free, size);
}

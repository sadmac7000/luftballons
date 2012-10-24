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
#include "drawlist.h"

/* We keep this out of shader.h to prevent circular dependency. */
void shader_set_vertex_attrs();

vbuf_t *current_vbuf = NULL;

/**
 * Create a new buffer object.
 *
 * size: Vertices the buffer should accomodate.
 * format: Segments that go in this buffer.
 **/
vbuf_t *
vbuf_create(size_t size, vbuf_fmt_t format)
{
	vbuf_t *ret;
	GLuint handle;
	GLenum error;
	GLsizeiptr byte_size = vbuf_fmt_vert_size(format);

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
	ret->format = format;
	ret->refcount = 0;

	intervals_init(&ret->free);
	interval_set(&ret->free, 0, size);

	list_init(&ret->drawlist_link);

	return ret;
}

/**
 * Setup the vertex attribute named for the attribute handle given.
 **/
void
vbuf_setup_vertex_attribute(const char *name, GLint handle)
{
	size_t position = 0;
	size_t elems;
	vbuf_fmt_t iter;
	const char *sname;
	size_t size;
	GLenum type;
	int found = 0;

	if (! current_vbuf)
		return;

	iter = current_vbuf->format;

	while (vbuf_fmt_pop_segment(&iter, &elems, &type, &sname, &size)) {
		if (! strcmp(sname, name)) {
			found = 1;
			break;
		}

		position += size * current_vbuf->vert_count;
	}

	if (! found) {
		glDisableVertexAttribArray(handle);
		return;
	}

	glEnableVertexAttribArray(handle);
	glVertexAttribPointer(handle, elems, type, GL_FALSE, 0,
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

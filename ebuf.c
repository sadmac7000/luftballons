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

#include "ebuf.h"
#include "util.h"

ebuf_t *current_ebuf = NULL;

/**
 * Bind an element buffer. Don't check to see if it's bound first.
 **/
static void
ebuf_do_activate(ebuf_t *buffer)
{
	current_ebuf = buffer;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->gl_handle);
}

/**
 * Create a new buffer object.
 *
 * size: Indices the buffer should accomodate.
 **/
ebuf_t *
ebuf_create(size_t size)
{
	ebuf_t *ret = xmalloc(sizeof(ebuf_t));
	GLuint handle;
	GLenum error;

	if (! size)
		return NULL;

	glGenBuffers(1, &handle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(uint16_t), NULL,
		     GL_STATIC_DRAW);
	error = glGetError();

	if (current_ebuf)
		ebuf_do_activate(current_ebuf);

	if (error != GL_NO_ERROR) {
		if (error == GL_OUT_OF_MEMORY)
			return NULL;

		errx(1, "Unexpected OpenGL error allocating buffer");
	}

	ret->gl_handle = handle;
	ret->size = size;
	ret->refcount = 0;

	intervals_init(&ret->free);
	interval_set(&ret->free, 0, size);

	list_init(&ret->drawlist_link);

	return ret;
}

/**
 * Increase a buffer's refcount.
 **/
void
ebuf_grab(ebuf_t *buffer)
{
	buffer->refcount++;
}

/**
 * Decrease a buffer's refcount. If the count becomes zero, free it.
 **/
void
ebuf_ungrab(ebuf_t *buffer)
{
	if (--buffer->refcount)
		return;

	list_remove(&buffer->drawlist_link);

	if (current_ebuf == buffer) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		current_ebuf = NULL;
	}

	glDeleteBuffers(1, &buffer->gl_handle);

	intervals_release(&buffer->free);
	free(buffer);
}

/**
 * Create a new free region and add it to a buffer's lists.
 **/
void
ebuf_drop_data(ebuf_t *buffer, size_t offset, size_t size)
{
	interval_set(&buffer->free, offset, size);
}

/**
 * Allocate a region of a buffer.
 **/
void
ebuf_alloc_region(ebuf_t *buffer, size_t offset, size_t size)
{
	interval_unset(&buffer->free, offset, size);
}

/**
 * Bind an element buffer.
 **/
void
ebuf_activate(ebuf_t *buffer)
{
	if (current_ebuf == buffer)
		return;

	ebuf_do_activate(buffer);
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
ebuf_locate_free_space(ebuf_t *buffer, size_t size)
{
	return interval_find(&buffer->free, size);
}


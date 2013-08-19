/**
 * Copyright © 2013 Casey Dahlin
 *
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version/
 *
 * Luftballons is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
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
	CHECK_GL;
}

/**
 * Decrease a buffer's refcount. If the count becomes zero, free it.
 **/
static void
ebuf_destructor(void *buffer_)
{
	ebuf_t *buffer = buffer_;

	if (current_ebuf == buffer) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		current_ebuf = NULL;
	}

	glDeleteBuffers(1, &buffer->gl_handle);

	intervals_release(&buffer->free);
	free(buffer);
	CHECK_GL;
}

/**
 * Create a new buffer object.
 *
 * size: Indices the buffer should accomodate.
 **/
ebuf_t *
ebuf_create(size_t size)
{
	ebuf_t *ret;
	GLuint handle;
	int memfail;

	if (! size)
		return NULL;

	glGenBuffers(1, &handle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(uint16_t), NULL,
		     GL_STATIC_DRAW);
	memfail = CHECK_GL_MEM;

	if (current_ebuf)
		ebuf_do_activate(current_ebuf);

	if (memfail)
		return NULL;

	ret = xmalloc(sizeof(ebuf_t));
	ret->gl_handle = handle;
	ret->size = size;

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, ebuf_destructor, ret);

	intervals_init(&ret->free);
	interval_set(&ret->free, 0, size);

	return ret;
}

/**
 * Increase a buffer's refcount.
 **/
void
ebuf_grab(ebuf_t *buffer)
{
	refcount_grab(&buffer->refcount);
}

/**
 * Decrease a buffer's refcount. If the count becomes zero, free it.
 **/
void
ebuf_ungrab(ebuf_t *buffer)
{
	refcount_ungrab(&buffer->refcount);
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


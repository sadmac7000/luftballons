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
	ret->regions_sz = xmalloc(sizeof(buf_region_t *));

	ret->regions_off = xmalloc(sizeof(buf_region_t *));

	ret->region_count = 1;
	ret->regions_sz[0] = xmalloc(sizeof(buf_region_t));
	ret->regions_off[0] = ret->regions_sz[0];
	ret->regions_sz[0]->start = 0;
	ret->regions_sz[0]->size = size;

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
	size_t i;

	if (--buffer->refcount)
		return;

	if (current_array_buffer == buffer) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		current_array_buffer = NULL;
	}

	glDeleteBuffers(1, &buffer->gl_handle);

	for (i = 0; i < buffer->region_count; i++)
		free(buffer->regions_sz[i]);

	free(buffer->regions_sz);
	free(buffer->regions_off);
	free(buffer);
}

/**
 * Find an index into the size-sorted free list of an object that has the given
 * size. Will return the next consecutive item if there is none.
 **/
static size_t
buffer_bisect_regions_sz(buffer_t *buffer, size_t size)
{
	size_t pos;
	size_t start = 0;
	size_t end = buffer->region_count;

	buf_region_t *cmp;

	for (;;) {
		pos = (start + end) / 2;
		cmp = buffer->regions_sz[pos];

		if (cmp->size < size)
			end = pos;
		else if (cmp->size > size && pos != start)
			start = pos;
		else if (cmp->size == size)
			return pos;
		else if (pos == start && end == buffer->region_count)
			return buffer->region_count;
		else
			return pos;
	}
}

/**
 * Remove a free region from a buffer.
 *
 * buffer: The buffer to operate on.
 * offset_idx: Position in the offset list where the free region can be found.
 **/
static void
buffer_remove_free_region(buffer_t *buffer, size_t offset_idx)
{
	size_t pos;

	buf_region_t *region = buffer->regions_off[offset_idx];

	memmove(&buffer->regions_off[offset_idx],
		&buffer->regions_off[offset_idx + 1],
		(buffer->region_count - offset_idx - 1) *
		sizeof(buf_region_t *));

	pos = buffer_bisect_regions_sz(buffer, region->size);

	if (buffer->regions_sz[pos]->size != region->size)
		errx(1, "Item missing from buffer region size list");

	while (pos && buffer->regions_sz[pos - 1]->size == region->size)
		pos--;

	while(buffer->regions_sz[pos] != region)
		pos++;
	
	memmove(&buffer->regions_sz[offset_idx],
		&buffer->regions_sz[offset_idx + 1],
		(buffer->region_count - offset_idx - 1) *
		sizeof(buf_region_t *));

	buffer->region_count--;
}

/**
 * Reposition an item in the size array because it has changed size.
 **/
static void
buffer_reposition_in_size_array(buffer_t *buffer, size_t idx)
{
	size_t idx_new = idx;
	buf_region_t *region = buffer->regions_sz[idx];

	while (idx && region->size < buffer->regions_sz[idx - 1]->size)
		idx_new--;

	while (idx < (buffer->region_count - 1) &&
	       region->size < buffer->regions_sz[idx + 1]->size)
		idx_new++;

	if (idx_new == idx)
		return;

	if (idx_new > idx)
		memmove(&buffer->regions_sz[idx], &buffer->regions_sz[idx + 1],
			(idx_new - idx) * sizeof(buf_region_t *));
	else
		memmove(&buffer->regions_sz[idx_new + 1], &buffer->regions_sz[idx_new],
			(idx - idx_new) * sizeof(buf_region_t *));

	buffer->regions_sz[idx_new] = region;
}

/**
 * Merge a region in a buffer with items on either side of it if there is no
 * space between them.
 *
 * buffer: Buffer to act on.
 * idx: Index in the offset list of the region to merge.
 **/
static void
buffer_do_merge(buffer_t *buffer, size_t idx)
{
	size_t count = 0;
	size_t end_size;
	buf_region_t *pos = buffer->regions_off[idx];
	buf_region_t *cmp;

	if (idx + 1 < buffer->region_count) {
		cmp = buffer->regions_off[idx + 1];

		if (cmp->start == pos->start + pos->size)
			count++;
	}

	if (idx > 0) {
		cmp = buffer->regions_off[idx - 1];

		if (cmp->start == pos->start + pos->size) {
			count++;
			idx--;
		}
	}

	if (! count)
		return;

	cmp = buffer->regions_off[idx + count];
	end_size = cmp->start - pos->start + cmp->size;

	for(;count; count--)
		buffer_remove_free_region(buffer, idx + count);

	pos->size = end_size;

	buffer_reposition_in_size_array(buffer, idx);
}

/**
 * Find an index into the offset-sorted free list of an object that has the
 * given offset. Will return the next consecutive item if there is none.
 **/
static size_t
buffer_bisect_regions_off(buffer_t *buffer, size_t offset)
{
	size_t pos;
	size_t start = 0;
	size_t end = buffer->region_count;

	buf_region_t *cmp;

	for (;;) {
		pos = (start + end) / 2;
		cmp = buffer->regions_off[pos];

		if (cmp->start < offset)
			end = pos;
		else if (cmp->start > offset && pos != start)
			start = pos;
		else if (cmp->start == start)
			return pos;
		else if (pos == start && end == buffer->region_count)
			return buffer->region_count;
		else
			return pos;
	}
}

/**
 * Make sure the region lists are big enough to accomodate one more.
 **/
static void
buffer_expand_region_space(buffer_t *buffer)
{
	if (buffer->region_count & (buffer->region_count - 1))
		return;

	buffer->regions_sz = xrealloc(buffer->regions_sz, 2 *
				      buffer->region_count *
				      sizeof(buf_region_t *));
	buffer->regions_off = xrealloc(buffer->regions_off, 2 *
				       buffer->region_count *
				       sizeof(buf_region_t *));
}


/**
 * Create a new free region and add it to a buffer's lists.
 **/
void
buffer_drop_data(buffer_t *buffer, size_t offset, size_t size)
{
	size_t sz_pos;
	size_t off_pos;

	buf_region_t *new = xmalloc(sizeof(buf_region_t));
	buf_region_t *cmp;

	new->start = offset;
	new->size = size;

	off_pos = buffer_bisect_regions_off(buffer, new->start);

	if (off_pos > 0) {
		cmp = buffer->regions_off[off_pos - 1];

		if (cmp->start + cmp->size == offset) {
			cmp->size += size;
			buffer_do_merge(buffer, off_pos - 1);
			return;
		}
	}

	if (off_pos < buffer->region_count) {
		cmp = buffer->regions_off[off_pos];

		if (cmp->start == offset + size) {
			cmp->start = offset;
			buffer_do_merge(buffer, off_pos);
			return;
		}
	}

	sz_pos = buffer_bisect_regions_sz(buffer, new->size);

	buffer_expand_region_space(buffer);

	memmove(&buffer->regions_sz[sz_pos + 1], &buffer->regions_sz[sz_pos],
		(buffer->region_count - sz_pos) * sizeof(buf_region_t *));
	memmove(&buffer->regions_off[off_pos + 1],
		&buffer->regions_off[off_pos],
		(buffer->region_count - off_pos) * sizeof(buf_region_t *));
	buffer->region_count++;

	buffer->regions_sz[sz_pos] = new; buffer->regions_off[off_pos] = new;
}

/**
 * Allocate a region of a buffer.
 **/
void
buffer_alloc_region(buffer_t *buffer, size_t offset, size_t size)
{
	size_t begin = 0;
	size_t end = buffer->region_count;
	size_t split;
	size_t create_size = 0;
	buf_region_t *pos;

	for (;;) {
		split = (begin + end) / 2;
		pos = buffer->regions_off[split];

		if (pos->start > offset) {
			end = split;
		} else if (pos->start + pos->size <= offset) {
			if (begin == split)
				errx(1,
				     "Buffer allocation outside free areas");
			begin = split;
		} else {
			break;
		}
	}

	if ((pos->start + pos->size) > offset + size)
		errx(1, "Buffer allocation extends past free area");

	if (pos->start == offset && pos->size == size) {
		buffer_remove_free_region(buffer, split);
		return;
	}

	if (pos->start == offset) {
		pos->start += size;
		return;
	}

	create_size = pos->size - size - (offset - pos->start);
	pos->size = (offset - pos->start);

	buffer_reposition_in_size_array(buffer, split);
	if (create_size)
		buffer_drop_data(buffer, pos->start + pos->size + size,
				 create_size);
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
	buf_region_t *pos;

	if (! buffer->region_count)
		return -1;

	pos = buffer->regions_sz[0];

	if (pos->size < size)
		return -1;

	return pos->start + pos->size - size;
}

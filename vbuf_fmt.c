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

#include <err.h>
#include <stdint.h>
#include <string.h>
#include <GL/glut.h>

#include "vbuf_fmt.h"
#include "util.h"

#ifndef VBUF_SEGMENT_HTABLE_SIZE
#define VBUF_SEGMENT_HTABLE_SIZE 67 /* Next prime from 64 */
#endif

/* Just an unused segment ID to fill empty spots in the hash table. */
#define VBUF_SEGMENT_UNUSED 255

/**
 * Table of segments. The index maps them to types.
 **/
static struct vbuf_segment {
	size_t elems;
	GLenum type;
	const char *name;
} vbuf_segments[64] = {{0,0,NULL}};

/**
 * Highest used entry in vbuf_segments.
 **/
static vbuf_seg_id_t vbuf_segments_sz = 0;

/**
 * Hash table for looking up segment IDs based on their properties.
 **/
static vbuf_seg_id_t vbuf_segment_htable[VBUF_SEGMENT_HTABLE_SIZE] = {
	[0 ... VBUF_SEGMENT_HTABLE_SIZE-1] = VBUF_SEGMENT_UNUSED,
};

/**
 * Mapping of GL types onto sizes.
 **/
static size_t vbuf_type_sizes[] = {
	[GL_BYTE] = sizeof(GLbyte),
	[GL_UNSIGNED_BYTE] = sizeof(GLubyte),
	[GL_SHORT] = sizeof(GLshort),
	[GL_UNSIGNED_SHORT] = sizeof(GLushort),
	[GL_INT] = sizeof(GLint),
	[GL_UNSIGNED_INT] = sizeof(GLuint),
	[GL_FLOAT] = sizeof(GLfloat),
	[GL_DOUBLE] = sizeof(GLdouble),

	/* http://www.opengl.org/registry/specs/NV/half_float.txt */
	[GL_HALF_FLOAT] = 2,

	/* http://www.opengl.org/registry/specs/OES/OES_fixed_point.txt */
	[GL_FIXED] = 4,

	[GL_INT_2_10_10_10_REV] = sizeof(int),
	[GL_UNSIGNED_INT_2_10_10_10_REV] = sizeof(int),
};

/* Length of vbuf_type_sizes */
static size_t vbuf_type_sizes_max = sizeof(vbuf_type_sizes) /
	sizeof(size_t) - 1;

/**
 * Compute a hash value for a vbuf_segment struct.
 **/
static uint64_t
vbuf_segment_hash(struct vbuf_segment *seg)
{
	static const uint64_t fnv_prime = 0x100000001b3ULL;
	static const uint64_t fnv_osb = 0xcbf29ce484222325ULL;

	size_t len = strlen(seg->name);
	size_t i;
	uint64_t ret = fnv_osb;

	for (i = 0; i < len; i++) {
		ret ^= seg->name[i];
		ret *= fnv_prime;
	}

	for (i = 0; i < sizeof(size_t); i++) {
		ret ^= ((char *)&seg->elems)[i];
		ret *= fnv_prime;
	}

	for (i = 0; i < sizeof(GLenum); i++) {
		ret ^= ((char *)&seg->type)[i];
		ret *= fnv_prime;
	}

	return ret;
}

/**
 * Look up a segment ID given a vbuf_segment struct
 **/
static vbuf_seg_id_t
vbuf_segment_lookup(struct vbuf_segment *seg)
{
	uint64_t hash = vbuf_segment_hash(seg);
	vbuf_seg_id_t ret =
		vbuf_segment_htable[hash % VBUF_SEGMENT_HTABLE_SIZE];

	for (;ret != VBUF_SEGMENT_UNUSED;
	     ret = (ret + 1) % VBUF_SEGMENT_HTABLE_SIZE) {
		if (vbuf_segments[ret].type != seg->type)
			continue;

		if (vbuf_segments[ret].elems != seg->elems)
			continue;

		if (strcmp(vbuf_segments[ret].name, seg->name))
			continue;

		return ret;
	}

	if (vbuf_segments_sz == 64)
		errx(1, "Ran out of vbuf segment descriptors");

	seg->name = xstrdup(seg->name);
	ret = vbuf_segments_sz++;
	vbuf_segments[ret] = *seg;
	vbuf_segment_htable[hash % VBUF_SEGMENT_HTABLE_SIZE] = ret;

	return ret;
}

/**
 * Size of a vertex within in a given segment.
 **/
static size_t
vbuf_segment_size(struct vbuf_segment *seg)
{
	size_t ret = 0;

	if (seg->type <= vbuf_type_sizes_max)
		ret = vbuf_type_sizes[seg->type];

	if (ret)
		return ret * seg->elems;

	errx(1, "Tried to get size of invalid GL type");
}

/**
 * Add a segment with the given properties to the given vbuf format.
 **/
void
vbuf_fmt_add(vbuf_fmt_t *fmt, const char *name, size_t elems, GLenum type)
{
	struct vbuf_segment seg = { elems, type, name };
	vbuf_seg_id_t id = vbuf_segment_lookup(&seg);

	*fmt |= 1 << id;
}

/**
 * Remove one of the segments from the given vbuf_fmt_t and return it.
 **/
static struct vbuf_segment *
vbuf_fmt_do_pop_segment(vbuf_fmt_t *iter)
{
	vbuf_seg_id_t id = 0;
	vbuf_fmt_t mask = 1;

	if (! *iter)
		return NULL;

	while (! (*iter & mask)) {
		mask <<= 1;
		id++;
	}

	*iter &= ~mask;

	return &vbuf_segments[id];
}

/**
 * Find the size of a vertex in the given format.
 **/
size_t
vbuf_fmt_vert_size(vbuf_fmt_t fmt)
{
	struct vbuf_segment *seg;
	size_t ret = 0;

	while ((seg = vbuf_fmt_do_pop_segment(&fmt)))
		ret += vbuf_segment_size(seg);

	return ret;
}

/**
 * Remove one of the segments from the given vbuf_fmt_t and return its
 * parameters.
 **/
int
vbuf_fmt_pop_segment(vbuf_fmt_t *iter, size_t *elems, GLenum *type,
		     const char **name, size_t *size)
{
	struct vbuf_segment *seg = vbuf_fmt_do_pop_segment(iter);

	if (! seg)
		return 0;

	if (elems)
		*elems = seg->elems;

	if (type)
		*type = seg->type;

	if (name)
		*name = seg->name;

	if (size)
		*size = vbuf_segment_size(seg);

	return 1;
}

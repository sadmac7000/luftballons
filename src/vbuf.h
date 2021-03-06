/**
 * Copyright © 2013 Casey Dahlin
 *
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Luftballons is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Luftballons.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef VBUF_H
#define VBUF_H

#include <GL/gl.h>

#include "interval.h"
#include "vbuf_fmt.h"
#include "refcount.h"
#include "util.h"

/**
 * Wrapper around an OpenGL buffer object for vertex data buffers.
 *
 * gl_handle: OpenGL handle for the buffer.
 * format: Type of data in each segment.
 * vert_size: size of the data for a vertex in bytes.
 * vert_count: total verts in this buffer.
 * refcount: Reference counting.
 * free: Free space tracking.
 **/
typedef struct vbuf {
	GLuint gl_handle;

	vbuf_fmt_t format;
	size_t vert_size;
	size_t vert_count;

	refcounter_t refcount;

	intervals_t free;
} vbuf_t;

#ifdef __cplusplus
extern "C" {
#endif

vbuf_t *vbuf_create(size_t size, vbuf_fmt_t format);
void vbuf_grab(vbuf_t *buffer);
void vbuf_ungrab(vbuf_t *buffer);
void vbuf_drop_data(vbuf_t *buffer, size_t offset, size_t size);
void vbuf_activate(vbuf_t *buffer);
void vbuf_alloc_region(vbuf_t *buffer, size_t offset, size_t size);
ssize_t vbuf_locate_free_space(vbuf_t *buffer, size_t size);
void vbuf_setup_vertex_attribute(const char *name, GLint handle);

#ifdef __cplusplus
}
#endif

#endif /* VBUF_H */


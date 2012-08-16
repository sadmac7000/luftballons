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

#ifndef EBUF_H
#define EBUF_H

#include <GL/glut.h>

#include "interval.h"

/**
 * Wrapper around an OpenGL buffer object for element buffers.
 *
 * gl_handle: OpenGL handle for the buffer.
 * size: total verts in this buffer.
 * refcount: Reference counting.
 * free: Free space tracking.
 **/
typedef struct ebuf {
	GLuint gl_handle;
	size_t size;
	unsigned int refcount;
	intervals_t free;
} ebuf_t;

#ifdef __cplusplus
extern "C" {
#endif

ebuf_t *ebuf_create(size_t size);
void ebuf_grab(ebuf_t *buffer);
void ebuf_ungrab(ebuf_t *buffer);
void ebuf_drop_data(ebuf_t *buffer, size_t offset, size_t size);
void ebuf_activate(ebuf_t *buffer);
void ebuf_alloc_region(ebuf_t *buffer, size_t offset, size_t size);
ssize_t ebuf_locate_free_space(ebuf_t *buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* EBUF_H */


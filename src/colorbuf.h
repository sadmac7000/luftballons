/**
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

#ifndef COLORBUF_H
#define COLORBUF_H
#include <luftballons/colorbuf.h>

#include "texmap.h"
#include "refcount.h"

#define COLORBUF_CLEAR		LUFT_COLORBUF_CLEAR
#define COLORBUF_CLEAR_DEPTH	LUFT_COLORBUF_CLEAR_DEPTH
#define COLORBUF_CLEAR_STENCIL	LUFT_COLORBUF_CLEAR_STENCIL
#define COLORBUF_DEPTH		LUFT_COLORBUF_DEPTH
#define COLORBUF_STENCIL	LUFT_COLORBUF_STENCIL
#define COLORBUF_VALID_FLAGS    LUFT_COLORBUF_VALID_FLAGS  

/* Internal flags */
#define COLORBUF_RENDERBUF_HAS_STORAGE	0x100
#define COLORBUF_INITIALIZED		0x200
#define COLORBUF_NEEDS_CLEAR		0x400

/**
 * A set of color buffer targets.
 *
 * num_colorbufs, colorbufs: The set of buffers.
 * colorbuf_attach_pos: Parallel to `colorbufs`. Attachment indicies.
 * framebuf: OpenGL framebuffer handle.
 * clear_color: Color to clear to if we clear.
 * clear_depth: Depth to clear to if we clear depth.
 * clear_stencil: Stencil clear-index if we clear that.
 * depth_texmap: A texmap to use as a depth buffer.
 * flags: Various flags.
 * autodepth: Our autodepth buffer.
 * refcount: Reference counter.
 **/
typedef struct colorbuf {
	size_t num_colorbufs;
	texmap_t **colorbufs;
	size_t *colorbuf_attach_pos;

	GLuint framebuf;

	float clear_color[4];
	float clear_depth;
	int clear_stencil;

	texmap_t *depth_texmap;

	unsigned int flags;

	GLuint autodepth;

	refcounter_t refcount;
} colorbuf_t;

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(colorbuf_create);
API_DECLARE(colorbuf_grab);
API_DECLARE(colorbuf_ungrab);
API_DECLARE(colorbuf_set_buf);
API_DECLARE(colorbuf_clear);
API_DECLARE(colorbuf_copy);
API_DECLARE(colorbuf_clear_color);
API_DECLARE(colorbuf_clear_depth);
API_DECLARE(colorbuf_clear_stencil);
API_DECLARE(colorbuf_init_output);
API_DECLARE(colorbuf_set_output_geom);
API_DECLARE(colorbuf_max_bufs);
API_DECLARE(colorbuf_set_depth_buf);

void colorbuf_prep(colorbuf_t *colorbuf);

#ifdef __cplusplus
}
#endif

#endif /* COLORBUF_H */

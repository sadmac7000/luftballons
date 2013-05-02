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

#ifndef COLORBUF_H
#define COLORBUF_H

#include "texmap.h"
#include "refcount.h"

/**
 * Flag value to indicate if we should clear this buffer.
 **/
#define COLORBUF_CLEAR		0x1
#define COLORBUF_CLEAR_DEPTH	0x2
#define COLORBUF_CLEAR_STENCIL	0x4
#define COLORBUF_AUTO_DEPTH	0x8
#define COLORBUF_STENCIL	0x10
#define COLORBUF_VALID_FLAGS    0x1f

/* Internal flags */
#define COLORBUF_RENDERBUF_HAS_STORAGE	0x100
#define COLORBUF_INITIALIZED		0x200

/**
 * A set of color buffer targets.
 *
 * num_colorbufs, colorbufs: The set of buffers.
 * deps_total: Number of dependencies this buffer has.
 * deps_complete: Number of dependencies which have been satisfied right now.
 * ready_callback: Callback to run when all deps are complete.
 * ready_callback_data: Data to pass to the ready callback.
 * clear_color: Color to clear to if we clear.
 * clear_depth: Depth to clear to if we clear depth.
 * clear_stencil: Stencil clear-index if we clear that.
 * flags: Various flags.
 * autodepth: Our autodepth buffer.
 * refcount: Reference counter.
 **/
typedef struct colorbuf {
	size_t num_colorbufs;
	texmap_t **colorbufs;

	size_t deps_total;
	size_t deps_complete;

	void (*ready_callback)(void *);
	void *ready_callback_data;

	float clear_color[4];
	float clear_depth;
	int clear_stencil;

	unsigned int flags;

	GLuint autodepth;

	refcounter_t refcount;
} colorbuf_t;

#ifdef __cplusplus
extern "C" {
#endif

colorbuf_t *colorbuf_create(unsigned int flags);
void colorbuf_grab(colorbuf_t *colorbuf);
void colorbuf_ungrab(colorbuf_t *colorbuf);
size_t colorbuf_append_buf(colorbuf_t *buf, texmap_t *texmap);
void colorbuf_dep_grab(colorbuf_t *colorbuf);
void colorbuf_dep_ungrab(colorbuf_t *colorbuf);
void colorbuf_complete_dep(colorbuf_t *colorbuf);
void colorbuf_invalidate(colorbuf_t *colorbuf);
void colorbuf_on_ready(colorbuf_t *colorbuf, void (*callback)(void *), void *data);
void colorbuf_prep(colorbuf_t *colorbuf);
void colorbuf_copy(colorbuf_t *in, colorbuf_t *out);
void colorbuf_clear_color(colorbuf_t *in, float color[4]);
void colorbuf_clear_depth(colorbuf_t *in, float depth);
void colorbuf_clear_stencil(colorbuf_t *in, int index);
void colorbuf_init_output(unsigned int flags);
void colorbuf_set_output_geom(size_t w, size_t h);
size_t colorbuf_max_bufs(void);

#ifdef __cplusplus
}
#endif

#endif /* COLORBUF_H */
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

#ifndef LUFT_COLORBUF_H
#define LUFT_COLORBUF_H

#include <luftballons/texmap.h>

/**
 * Flag value to indicate how we should manage this buffer.
 **/
#define LUFT_COLORBUF_CLEAR		0x1
#define LUFT_COLORBUF_CLEAR_DEPTH	0x2
#define LUFT_COLORBUF_CLEAR_STENCIL	0x4
#define LUFT_COLORBUF_DEPTH		0x8
#define LUFT_COLORBUF_STENCIL		0x10
#define LUFT_COLORBUF_VALID_FLAGS	0x1f

typedef struct colorbuf luft_colorbuf_t;

#ifdef __cplusplus
extern "C" {
#endif

luft_colorbuf_t *luft_colorbuf_create(unsigned int flags);
void luft_colorbuf_grab(luft_colorbuf_t *luft_colorbuf);
void luft_colorbuf_ungrab(luft_colorbuf_t *luft_colorbuf);
void luft_colorbuf_set_buf(luft_colorbuf_t *buf, size_t idx,
			   luft_texmap_t *texmap);
void luft_colorbuf_clear(luft_colorbuf_t *luft_colorbuf);
void luft_colorbuf_copy(luft_colorbuf_t *in, size_t in_idx,
			luft_colorbuf_t *out, size_t out_idx);
void luft_colorbuf_clear_color(luft_colorbuf_t *in, float color[4]);
void luft_colorbuf_clear_depth(luft_colorbuf_t *in, float depth);
void luft_colorbuf_clear_stencil(luft_colorbuf_t *in, int index);
void luft_colorbuf_init_output(unsigned int flags);
void luft_colorbuf_set_output_geom(size_t w, size_t h);
size_t luft_colorbuf_max_bufs(void);
void luft_colorbuf_set_depth_buf(luft_colorbuf_t *colorbuf,
				 luft_texmap_t *texmap);

#ifdef __cplusplus
}
#endif

#endif /* LUFT_COLORBUF_H */


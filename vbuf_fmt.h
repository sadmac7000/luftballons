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

#ifndef VBUF_FMT_H
#define VBUF_FMT_H

#include <stdint.h>

/**
 * A format for a vertex buffer. Each bit maps to a segment name, type and
 * size. If a bit is present, this buffer includes a segment matching that
 * description.
 **/
typedef uint64_t vbuf_fmt_t;

/**
 * A segment ID. Basically contains an index for a bit in vbuf_fmt_t.
 **/
typedef uint8_t vbuf_seg_id_t;

#ifdef __cplusplus
extern "C" {
#endif

void vbuf_fmt_add(vbuf_fmt_t *fmt, const char *name, size_t elems,
		  GLenum type);
size_t vbuf_fmt_vert_size(vbuf_fmt_t fmt);
int vbuf_fmt_pop_segment(vbuf_fmt_t *iter, size_t *elems, GLenum *type,
			 const char **name, size_t *size);

#ifdef __cplusplus
}
#endif

#endif /* VBUF_FMT_H */

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

#ifndef TEXMAP_H
#define TEXMAP_H
#include <luftballons/texmap.h>

#include <GL/gl.h>

#include <config.h>

#include "refcount.h"
#include "util.h"

#define TEXMAP_COMPRESSED LUFT_TEXMAP_COMPRESSED
#define TEXMAP_FLOAT32 LUFT_TEXMAP_FLOAT32
#define TEXMAP_DEPTH LUFT_TEXMAP_DEPTH
#define TEXMAP_STENCIL LUFT_TEXMAP_STENCIL

#define TEXMAP_WRAP_R LUFT_TEXMAP_WRAP_R
#define TEXMAP_WRAP_S LUFT_TEXMAP_WRAP_S
#define TEXMAP_WRAP_T LUFT_TEXMAP_WRAP_T

#define TEXMAP_WRAP_CLAMP LUFT_TEXMAP_WRAP_CLAMP
#define TEXMAP_WRAP_MIRROR LUFT_TEXMAP_WRAP_MIRROR
#define TEXMAP_WRAP_REPEAT LUFT_TEXMAP_WRAP_REPEAT

#define TEXMAP_INTERP_NONE LUFT_TEXMAP_INTERP_NONE
#define TEXMAP_INTERP_NEAREST LUFT_TEXMAP_INTERP_NEAREST
#define TEXMAP_INTERP_LINEAR LUFT_TEXMAP_INTERP_LINEAR

typedef luft_texmap_interp_t texmap_interp_t;

/**
 * A 2D texture map, usually loaded from a file.
 *
 * map: OpenGL texture ID
 * sampler: OpenGL sampler object
 * flags: Misc flags.
 * base_mip: Base mip map level.
 * max_mip: Maximum mip map level.
 * initialized: Bit field of which mip map levels are initialized.
 * w, h: Width and height
 * texture_unit: What texture unit this texture was assigned.
 * refcount: Number of references to this texture
 **/
typedef struct texmap {
	GLuint map;
	GLuint sampler;
	unsigned int flags;
	size_t base_mip;
	size_t max_mip;
	uint64_t *initialized;
	size_t w, h;
	GLuint texture_unit;
	refcounter_t refcount;
} texmap_t;

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(texmap_create);
API_DECLARE(texmap_load_image);
API_DECLARE(texmap_init_blank);
API_DECLARE(texmap_set_mag);
API_DECLARE(texmap_set_min);
API_DECLARE(texmap_set_wrap);
API_DECLARE(texmap_grab);
API_DECLARE(texmap_ungrab);

size_t texmap_get_texture_unit(texmap_t *texmap);
void texmap_end_unit_generation(void);

/* Prototypes for image format handlers */
#ifdef HAVE_PNG
extern int texmap_load_image_png(texmap_t *map, GLint level, int fd,
				 const char *path);
#else
static inline int
texmap_load_image_png(texmap_t *map, GLint level, int fd, const char *path)
{
	(void)map;
	(void)level;
	(void)fd;
	(void)path;

	return 0;
}
#endif

extern int texmap_load_image_tiff(texmap_t *map, GLint level, int fd,
				  const char *path);

#ifdef __cplusplus
}
#endif

#endif /* TEXMAP_H */

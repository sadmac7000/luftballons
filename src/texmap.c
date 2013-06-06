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

#include <err.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "texmap.h"
#include "util.h"

/* Prototypes for image format handlers */
extern int texmap_load_image_png(texmap_t *map, GLint level, int fd,
				 const char *path);
extern int texmap_load_image_tiff(texmap_t *map, GLint level, int fd,
				  const char *path);

/* Texture unit assignment */
texmap_t **units = NULL;
GLuint *generation = NULL;
size_t generation_size = 0;
size_t max_units = 0;

/**
 * Mark a texmap as initialized for a given level.
 **/
static void
texmap_mark_init(texmap_t *map, int level)
{
	uint64_t *word;
	uint64_t mask;

	level -= map->base_mip;

	word = &map->initialized[level / 64];
	mask = 1 << (level % 64);

	if ((*word) & mask)
		errx(1, "Each texmap level can be inititalized only once");

	(*word) |= mask;
}

/**
 * Load image from a file into a texture map.
 *
 * map: Map to load in to.
 * path: File to load.
 * level: Mipmap level to load.
 **/
void
texmap_load_image(texmap_t *map, const char *path, int level)
{
	int fd = open(path, O_RDONLY | O_CLOEXEC);

	texmap_mark_init(map, level);

	if (fd < 0)
		err(1, "Could not open image file %s", path);

	if (texmap_load_image_png(map, level, fd, path))
		return;

	lseek(fd, 0, SEEK_SET);

	if (texmap_load_image_tiff(map, level, fd, path))
		return;

	errx(1, "Unrecognized image format for %s", path);
}
EXPORT(texmap_load_image);

/**
 * Destroy a texmap
 **/
static void
texmap_destructor(void *texmap_)
{
	texmap_t *texmap = texmap_;

	if (units && units[texmap->texture_unit] == texmap)
		units[texmap->texture_unit] = NULL;

	glDeleteSamplers(1, &texmap->sampler);
	glDeleteTextures(1, &texmap->map);
	free(texmap);
	CHECK_GL;
}

/**
 * Create a texture map, and load it with contents from a file.
 *
 * base_level: Base mipmap level
 * max_level: Max mipmap level
 * flags: Texture flags
 **/
texmap_t *
texmap_create(size_t base_level, size_t max_level, unsigned int flags)
{
	texmap_t *map = xmalloc(sizeof(texmap_t));
	size_t num_mips = max_level - base_level;
	size_t num_bit_words = (num_mips + 63) / 64;

	if (max_level < base_level)
		errx(1, "Max mip map level can't be less than base");

	if (flags & ~(TEXMAP_COMPRESSED | TEXMAP_FLOAT32 |
		      TEXMAP_DEPTH | TEXMAP_STENCIL))
		errx(1, "Unrecognized flags for texmap");

	if ((flags & TEXMAP_COMPRESSED) && (flags & TEXMAP_FLOAT32))
		errx(1, "Cannot create a floating point compressed texture");

	if ((flags & TEXMAP_STENCIL) && !(flags & TEXMAP_DEPTH))
		errx(1, "Stencil textures must also be depth textures.");

	if ((flags & TEXMAP_COMPRESSED) && (flags & TEXMAP_DEPTH))
		errx(1, "Cannot create a compressed depth texture");

	glGenTextures(1, &map->map);
	glGenSamplers(1, &map->sampler);

	map->base_mip = base_level;
	map->max_mip = max_level;
	map->texture_unit = 0;
	map->initialized = xcalloc(num_bit_words, sizeof(uint64_t));
	texmap_get_texture_unit(map);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, base_level);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max_level);

	refcount_init(&map->refcount);
	refcount_add_destructor(&map->refcount, texmap_destructor, map);

	map->flags = flags;

	CHECK_GL;
	return map;
}
EXPORT(texmap_create);

/**
 * Increase the refcount of a texmap.
 **/
void
texmap_grab(texmap_t *texmap)
{
	refcount_grab(&texmap->refcount);
}
EXPORT(texmap_grab);

/**
 * Decrease the refcount of a texmap.
 **/
void
texmap_ungrab(texmap_t *texmap)
{
	refcount_ungrab(&texmap->refcount);
}
EXPORT(texmap_ungrab);

/**
 * Set magnification mode for this texmap.
 *
 * map: Map to operate on.
 * interp: Interpolation mode to use.
 **/
void
texmap_set_mag(texmap_t *map, texmap_interp_t interp)
{
	if (interp == TEXMAP_INTERP_NONE)
		errx(1, "Magnification interpolation cannot be none");
	else if (interp == TEXMAP_INTERP_NEAREST)
		glSamplerParameteri(map->sampler, GL_TEXTURE_MAG_FILTER,
				    GL_NEAREST);
	else if (interp == TEXMAP_INTERP_LINEAR)
		glSamplerParameteri(map->sampler, GL_TEXTURE_MAG_FILTER,
				    GL_LINEAR);
	else
		errx(1, "Magnification interpolation must be a valid "
		     "interpolation constant");

	CHECK_GL;
}
EXPORT(texmap_set_mag);

/**
 * Set minification mode for this texmap.
 *
 * map: Map to operate on.
 * local: Interpolation to use sampling a single mip map.
 * mip: Interpolation to use between samples of multiple mip maps.
 **/
void
texmap_set_min(texmap_t *map, texmap_interp_t local, texmap_interp_t mip)
{
	if (local == TEXMAP_INTERP_NONE)
		errx(1, "Local interpolation cannot be none");

	if (local == TEXMAP_INTERP_NEAREST &&
	    mip == TEXMAP_INTERP_NONE)
		glSamplerParameteri(map->sampler, GL_TEXTURE_MIN_FILTER,
				    GL_NEAREST);
	else if (local == TEXMAP_INTERP_NEAREST &&
	    mip == TEXMAP_INTERP_NEAREST)
		glSamplerParameteri(map->sampler, GL_TEXTURE_MIN_FILTER,
				    GL_NEAREST_MIPMAP_NEAREST);
	else if (local == TEXMAP_INTERP_NEAREST &&
	    mip == TEXMAP_INTERP_LINEAR)
		glSamplerParameteri(map->sampler, GL_TEXTURE_MIN_FILTER,
				    GL_NEAREST_MIPMAP_LINEAR);
	else if (local == TEXMAP_INTERP_LINEAR &&
	    mip == TEXMAP_INTERP_NONE)
		glSamplerParameteri(map->sampler, GL_TEXTURE_MIN_FILTER,
				    GL_LINEAR);
	else if (local == TEXMAP_INTERP_LINEAR&&
	    mip == TEXMAP_INTERP_NEAREST)
		glSamplerParameteri(map->sampler, GL_TEXTURE_MIN_FILTER,
				    GL_LINEAR_MIPMAP_NEAREST);
	else if (local == TEXMAP_INTERP_LINEAR &&
	    mip == TEXMAP_INTERP_LINEAR)
		glSamplerParameteri(map->sampler, GL_TEXTURE_MIN_FILTER,
				    GL_LINEAR_MIPMAP_LINEAR);
	else
		errx(1, "Interpolation values must be valid texmap "
		     "interpolation constants");

	CHECK_GL;
}
EXPORT(texmap_set_min);

/**
 * Set the texture wrapping mode.
 *
 * map: Map to operate on.
 * wrap_axes: Bit field indicating which axes to set wrapping for.
 * wrap_type: Constant indicating what type of wrapping to do.
 **/
void
texmap_set_wrap(texmap_t *map, unsigned int wrap_axes, unsigned int wrap_type)
{
	GLint gl_value;

	if (wrap_axes & ~(TEXMAP_WRAP_R | TEXMAP_WRAP_S | TEXMAP_WRAP_T))
		errx(1, "Wrap axes must be an OR of one "
		     "or more wrap axis constants");

	switch (wrap_type) {
	case TEXMAP_WRAP_REPEAT:
		gl_value = GL_REPEAT;
		break;
	case TEXMAP_WRAP_MIRROR:
		gl_value = GL_MIRRORED_REPEAT;
		break;
	case TEXMAP_WRAP_CLAMP:
		gl_value = GL_CLAMP_TO_EDGE;
		break;
	default:
		errx(1, "Wrap type must be a valid wrap type constant");
	};

	if (wrap_axes & TEXMAP_WRAP_R)
		glSamplerParameteri(map->sampler, GL_TEXTURE_WRAP_R, gl_value);

	if (wrap_axes & TEXMAP_WRAP_S)
		glSamplerParameteri(map->sampler, GL_TEXTURE_WRAP_S, gl_value);

	if (wrap_axes & TEXMAP_WRAP_T)
		glSamplerParameteri(map->sampler, GL_TEXTURE_WRAP_T, gl_value);
}
EXPORT(texmap_set_wrap);

/**
 * Initialize a blank texmap.
 *
 * map: Which texmap to initialize.
 * level: Which level to initialize.
 * width, height: Pixel size of the map.
 * float32: If true, use a 32-bit floating point internal color format.
 **/
void
texmap_init_blank(texmap_t *map, int level, int width, int height)
{
	GLenum ifmt = GL_RGBA;
	GLenum fmt = GL_RGBA;
	GLenum colortype = GL_UNSIGNED_BYTE;

	texmap_mark_init(map, level);

	map->w = width;
	map->h = height;

	if (map->flags & TEXMAP_DEPTH) {
		fmt = GL_DEPTH_STENCIL;
		colortype = GL_UNSIGNED_INT_24_8;
		if (map->flags & TEXMAP_STENCIL)
			ifmt = GL_DEPTH24_STENCIL8;
		else
			ifmt = GL_DEPTH_COMPONENT32;
	}

	if (map->flags & TEXMAP_FLOAT32) {
		colortype = GL_FLOAT;

		if (!(map->flags & TEXMAP_DEPTH))
			ifmt = GL_RGBA32F;
		else if (map->flags & TEXMAP_STENCIL)
			ifmt = GL_DEPTH32F_STENCIL8;
		else
			ifmt = GL_DEPTH_COMPONENT32F;
	}

	if (map->flags & TEXMAP_COMPRESSED)
		ifmt = GL_COMPRESSED_RGBA;

	texmap_get_texture_unit(map);
	glTexImage2D(GL_TEXTURE_2D, level, ifmt, width, height, 0, fmt,
		     colortype, NULL);
	CHECK_GL;
}
EXPORT(texmap_init_blank);

/**
 * Merge two sorted segments of the generation list.
 **/
static void
texmap_generation_merge(size_t a, size_t b, size_t stop_b)
{
	size_t i;
	GLuint tmp;

	for(; b < stop_b; b++) {
		for (i = b; i > a; i--) {
			if (generation[i - 1] < generation[i])
				break;

			tmp = generation[i];
			generation[i] = generation[i - 1];
			generation[i - 1] = tmp;
		}
	}
}

/**
 * Sort the generation list.
 **/
static void
texmap_generation_sort(void)
{
	size_t i;
	size_t gap = 1;

	for (gap = 1; gap < generation_size; gap *= 2) {
		for (i = 0; i < generation_size; i += gap * 2) {
			if (i + gap * 2 <= generation_size)
				texmap_generation_merge(i, i + gap,
							i + 2 * gap);
			else
				texmap_generation_merge(i, i + gap,
							generation_size);
		}
	}
}

/**
 * Get the texture unit for this texmap.
 **/
size_t
texmap_get_texture_unit(texmap_t *texmap)
{
	GLint max;
	size_t i;

	if (! max_units) {
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max);
		max_units = (size_t)max;
	}

	if (units && units[texmap->texture_unit] == texmap) {
		glActiveTexture(GL_TEXTURE0 + texmap->texture_unit);
		glBindTexture(GL_TEXTURE_2D, texmap->map);
		return texmap->texture_unit;
	}

	generation = vec_expand(generation, generation_size);

	if (! units) {
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max);
		units = xcalloc(max, sizeof(texmap_t *));
	}

	for (i = 0; i < max_units; i++)
		if (units[i] == NULL)
			break;

	if (i == max_units) {
		texmap_generation_sort();

		/* This looks weird but it isn't. The first position where
		 * generation stops being a sorted list of integers starting
		 * from 0 is the free unit.
		 **/
		for (i = 0; i < generation_size; i++)
			if (generation[i] != i)
				break;
	}

	if (i == max_units)
		errx(1, "Out of texture units");

	generation[generation_size++] = i;
	units[i] = texmap;
	texmap->texture_unit = i;

	glActiveTexture(GL_TEXTURE0 + i);
	glBindTexture(GL_TEXTURE_2D, texmap->map);
	CHECK_GL;

	return texmap->texture_unit;
}

/**
 * Inform the texture unit allocator that it may now reallocate any previously
 * assigned texture unit whenever it sees fit.
 **/
void
texmap_end_unit_generation(void)
{
	generation_size = 0;
}

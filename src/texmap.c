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

	if (map->flags & TEXMAP_INITIALIZED)
		errx(1, "Tried to initialize texmap twice");

	map->flags |= TEXMAP_INITIALIZED;

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

	if (flags & ~(TEXMAP_COMPRESSED | TEXMAP_FLOAT32))
		errx(1, "Unrecognized flags for texmap");

	if ((flags & TEXMAP_COMPRESSED) && (flags & TEXMAP_FLOAT32))
		errx(1, "Cannot create a floating point compressed texture");

	glGenTextures(1, &map->map);
	glGenSamplers(1, &map->sampler);

	map->texture_unit = 0;
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
 * Set an OpenGL parameter for this texture map.
 **/
void
texmap_set_int_param(texmap_t *map, GLenum param, GLint value)
{
	glSamplerParameteri(map->sampler, param, value);
	CHECK_GL;
}
EXPORT(texmap_set_int_param);

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
	GLint ifmt = GL_RGBA;
	GLenum colortype = GL_UNSIGNED_BYTE;

	if (map->flags & TEXMAP_INITIALIZED)
		errx(1, "Tried to initialize texmap twice");

	map->flags |= TEXMAP_INITIALIZED;

	map->w = width;
	map->h = height;

	if (map->flags & TEXMAP_FLOAT32) {
		colortype = GL_FLOAT;
		ifmt = GL_RGBA32F;
	}

	if (map->flags & TEXMAP_COMPRESSED)
		ifmt = GL_COMPRESSED_RGBA;

	texmap_get_texture_unit(map);
	glTexImage2D(GL_TEXTURE_2D, level, ifmt, width, height, 0, GL_RGBA,
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

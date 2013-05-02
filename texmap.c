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

	if (fd < 0)
		err(1, "Could not open image file %s", path);

	if (texmap_load_image_png(map, level, fd, path))
		return;

	lseek(fd, 0, SEEK_SET);

	if (texmap_load_image_tiff(map, level, fd, path))
		return;

	errx(1, "Unrecognized image format for %s", path);
}

/**
 * Destroy a texmap
 **/
static void
texmap_destructor(void *texmap_)
{
	texmap_t *texmap = texmap_;

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
 * compress: Should we compress this texture.
 **/
texmap_t *
texmap_create(size_t base_level, size_t max_level, int compress)
{
	texmap_t *map = xmalloc(sizeof(texmap_t));

	glGenTextures(1, &map->map);
	glGenSamplers(1, &map->sampler);

	glBindTexture(GL_TEXTURE_2D, map->map);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, base_level);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max_level);

	refcount_init(&map->refcount);
	refcount_add_destructor(&map->refcount, texmap_destructor, map);
	if (compress)
		map->flags |= TEXMAP_COMPRESSED;

	CHECK_GL;
	return map;
}

/**
 * Increase the refcount of a texmap.
 **/
void
texmap_grab(texmap_t *texmap)
{
	refcount_grab(&texmap->refcount);
}

/**
 * Decrease the refcount of a texmap.
 **/
void
texmap_ungrab(texmap_t *texmap)
{
	refcount_ungrab(&texmap->refcount);
}

/**
 * Set an OpenGL parameter for this texture map.
 **/
void
texmap_set_int_param(texmap_t *map, GLenum param, GLint value)
{
	glSamplerParameteri(map->sampler, param, value);
	CHECK_GL;
}

/**
 * Initialize a blank texmap.
 **/
void
texmap_init_blank(texmap_t *map, int level, int width, int height)
{
	GLint ifmt = GL_RGBA;

	map->w = width;
	map->h = height;

	if (map->flags & TEXMAP_COMPRESSED)
		ifmt = GL_COMPRESSED_RGBA;

	glBindTexture(GL_TEXTURE_2D, map->map);
	glTexImage2D(GL_TEXTURE_2D, level, ifmt, width, height, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);
	CHECK_GL;
}

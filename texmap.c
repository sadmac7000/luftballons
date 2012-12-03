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
 * Create a texture map, and load it with contents from a file.
 *
 * base_level: Base mipmap level
 * max_level: Max mipmap level
 **/
texmap_t *
texmap_create(size_t base_level, size_t max_level)
{
	texmap_t *map = xmalloc(sizeof(texmap_t));

	glGenTextures(1, &map->map);
	glGenSamplers(1, &map->sampler);

	glBindTexture(GL_TEXTURE_2D, map->map);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, base_level);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max_level);

	return map;
}

/**
 * Set an OpenGL parameter for this texture map.
 **/
void
texmap_set_int_param(texmap_t *map, GLenum param, GLint value)
{
	glSamplerParameteri(map->sampler, param, value);
}

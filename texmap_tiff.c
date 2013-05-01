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

#include <stdlib.h>
#include <err.h>
#include <tiffio.h>

#include "texmap.h"
#include "util.h"

int
texmap_load_image_tiff(texmap_t *map, GLint level, int fd, const char *path)
{
	TIFF *img;
	img = TIFFFdOpen(fd, path, "r");
	uint32_t width, height;
	void *data;
	GLint ifmt = GL_RGBA;

	if (map->compressed)
		ifmt = GL_COMPRESSED_RGBA;

	if (! img)
		return 0;

	TIFFGetField(img, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(img, TIFFTAG_IMAGELENGTH, &height);

	data = xcalloc(width * height, sizeof(uint32_t));

	if (! TIFFReadRGBAImage(img, width, height, data, 0))
		errx(1, "Could not read TIFF");

	glBindTexture(GL_TEXTURE_2D, map->map);
	glTexImage2D(GL_TEXTURE_2D, level, ifmt, width, height, 0, GL_RGBA,
		     GL_UNSIGNED_BYTE, data);

	CHECK_GL;
	TIFFClose(img);
	free(data);

	map->w = width;
	map->h = height;

	return 1;
}

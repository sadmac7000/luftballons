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
#include <png.h>

#include "texmap.h"
#include "util.h"

/**
 * Hand our PNG data over to OpenGL.
 **/
static void
texmap_png_load_data(texmap_t *map, GLint level,
		     int width, int height, int color_type, void *data)
{
	GLenum gl_ifmt;
	GLenum gl_fmt;
	GLenum gl_type;

	switch (color_type) {
	case PNG_COLOR_TYPE_GRAY:
		gl_ifmt = GL_COMPRESSED_RED;
		gl_fmt = GL_RED;
		gl_type = GL_UNSIGNED_BYTE;
		break;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		gl_ifmt = GL_COMPRESSED_RG;
		gl_fmt = GL_RG;
		gl_type = GL_UNSIGNED_BYTE;
		break;
	case PNG_COLOR_TYPE_RGB:
		gl_ifmt = GL_COMPRESSED_RGB;
		gl_fmt = GL_RGB;
		gl_type = GL_UNSIGNED_BYTE;
		break;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		gl_ifmt = GL_COMPRESSED_RGBA;
		gl_fmt = GL_RGBA;
		gl_type = GL_UNSIGNED_BYTE;
		break;
	default:
		errx(1, "Encountered unknown PNG format\n");
	};

	glBindTexture(GL_TEXTURE_2D, map->map);
	glTexImage2D(GL_TEXTURE_2D, level, gl_ifmt, width, height, 0, gl_fmt,
		     gl_type, data);
}

/**
 * Load an image from a PNG file into a texture map.
 **/
int
texmap_load_image_png(texmap_t *map, GLint level, int fd)
{
	png_structp read_struct;
	png_infop info_struct;
	unsigned char header[8];
	int width;
	int height;
	png_byte color_type;
	png_byte bit_depth;
	int i;
	void *result;
	png_bytepp row_pointers;
	int stride;
	FILE *fp = fdopen(xdup(fd), "r"); /* I hate my life, and stdio */

	if (xread(fd, header, 8) < 8)
		return 0;

	if (png_sig_cmp(header, 0, 8))
		return 0;

	read_struct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (! read_struct)
		err(1, "Could not create libpng read struct");

	info_struct = png_create_info_struct(read_struct);

	if(! info_struct)
		err(1, "Could not create libpng info struct");

	if (setjmp(png_jmpbuf(read_struct)))
		err(1, "Could not get png data");

	png_init_io(read_struct, fp);
	png_set_sig_bytes(read_struct, 8);
	png_read_info(read_struct, info_struct);

	width = png_get_image_width(read_struct, info_struct);
	height = png_get_image_height(read_struct, info_struct);
	color_type = png_get_color_type(read_struct, info_struct);
	bit_depth = png_get_bit_depth(read_struct, info_struct);

	png_set_interlace_handling(read_struct);
	png_read_update_info(read_struct, info_struct);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		errx(1, "PNG palette color not supported");

	if (bit_depth != 8)
		errx(1, "PNG bit depth other than 8 not supported");

	if (setjmp(png_jmpbuf(read_struct)))
		err(1, "Could not read png");

	stride = png_get_rowbytes(read_struct, info_struct);
	row_pointers = xcalloc(height, sizeof(void *));
	result = xcalloc(height, stride);
	row_pointers[height - 1] = result;

	/* OpenGL likes its images "Upside down" from PNG's perspective */
	for (i = height - 2; i > 0; i--)
		row_pointers[i] = row_pointers[i + 1] + stride;

	png_read_image(read_struct, row_pointers);
	free(row_pointers);
	png_destroy_read_struct(&read_struct, &info_struct, NULL);

	texmap_png_load_data(map, level, width, height, color_type, result);
	free(result);
	fclose(fp);

	return 1;
}

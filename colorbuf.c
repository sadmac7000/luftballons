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

#include <GL/gl.h>

#include "colorbuf.h"
#include "util.h"

static colorbuf_t *current_colorbuf = NULL;

static GLuint framebuf;
static GLuint depth_attach;
static texmap_t **framebuf_maps = NULL;
static size_t framebuf_maps_size = 0;

/**
 * Destructor for colorbufs.
 **/
static void
colorbuf_destroy(void *buf_v)
{
	colorbuf_t *buf = buf_v;
	size_t i;

	for (i = 0; i < buf->num_colorbufs; i++)
		texmap_ungrab(buf->colorbufs[i]);

	free(buf->colorbufs);
	free(buf);
}

/**
 * Create a new colorbuf.
 **/
colorbuf_t *
colorbuf_create(void)
{
	colorbuf_t *ret = xcalloc(1, sizeof(colorbuf_t));

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, colorbuf_destroy, ret);

	return ret;
}

/**
 * Grab a colorbuf.
 **/
void
colorbuf_grab(colorbuf_t *colorbuf)
{
	refcount_grab(&colorbuf->refcount);
}

/**
 * Ungrab a colorbuf.
 **/
void
colorbuf_ungrab(colorbuf_t *colorbuf)
{
	refcount_ungrab(&colorbuf->refcount);
}

/**
 * Get the maximum number of color buffers we can have.
 **/
size_t
colorbuf_max_bufs(void)
{
	GLint result;

	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &result);

	CHECK_GL;
	return (size_t)result;
}

/**
 * Append a buf to a colorbuf.
 **/
size_t
colorbuf_append_buf(colorbuf_t *buf, texmap_t *texmap)
{
	if (buf->num_colorbufs == colorbuf_max_bufs())
		errx(1, "Platform does not support more than "
		     "%zu color buffers", colorbuf_max_bufs());

	buf->colorbufs = vec_expand(buf->colorbufs, buf->num_colorbufs);
	buf->colorbufs[buf->num_colorbufs++] = texmap;
	texmap_grab(texmap);

	return buf->num_colorbufs - 1;
}

/**
 * Grab a a colorbuf, inform it that we are a dependency.
 **/
void
colorbuf_dep_grab(colorbuf_t *colorbuf)
{
	colorbuf_grab(colorbuf);
	colorbuf->deps_total++;
}

/**
 * Ungrab a colorbuf, inform it that we no longer depend on it.
 **/
void
colorbuf_dep_ungrab(colorbuf_t *colorbuf)
{
	if (! colorbuf->deps_total)
		errx(1, "Colorbuf dependency count went negative");

	colorbuf->deps_total--;
	colorbuf_ungrab(colorbuf);
}

/**
 * Notify a colorbuf that one of its dependencies has been satisfied.
 **/
void
colorbuf_complete_dep(colorbuf_t *colorbuf)
{
	colorbuf->deps_complete++;

	if (colorbuf->deps_complete > colorbuf->deps_total)
		errx(1, "Dependency count mismatch in colorbuf");

	if (colorbuf->deps_complete == colorbuf->deps_total)
		colorbuf->ready_callback(colorbuf->ready_callback_data);
}

/**
 * Notify the colorbuf that its contents are no longer valid.
 **/
void
colorbuf_invalidate(colorbuf_t *colorbuf)
{
	colorbuf->deps_complete = 0;
}

/**
 * Allocate a color attachment in the framebuffer.
 **/
static int
colorbuf_alloc_framebuffer(texmap_t *map, GLenum *attach)
{
	size_t i;

	/* FIXME: If malloc reuses a memory segment we could destroy a texmap
	 * and then have a new one with the same address, which would appear to
	 * be in the framebuffer when it isn't.
	 *
	 * We can get away with freed textures that aren't reallocated because
	 * we only compare pointers, we don't dereference.
	 */
	for (i = 0; i < framebuf_maps_size && framebuf_maps[i] != map; i++);

	*attach = GL_COLOR_ATTACHMENT0 + i;

	if (i == framebuf_maps_size) {
		if (framebuf_maps_size == colorbuf_max_bufs())
			return -1;

		framebuf_maps = vec_expand(framebuf_maps, framebuf_maps_size);
		framebuf_maps[framebuf_maps_size++] = map;
		glFramebufferTexture2D(GL_FRAMEBUFFER, *attach, GL_TEXTURE_2D,
				       map->map, 0);
		CHECK_GL;
	}

	return 0;
}

/**
 * Check the status of the framebuffer.
 **/
static void
colorbuf_check_status(void)
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status == GL_FRAMEBUFFER_COMPLETE)
		return;

	if (status == GL_FRAMEBUFFER_UNDEFINED)
		errx(1, "Wat. OpenGL got "
		     "GL_FRAMEBUFFER_UNDEFINED when it shouldn't");

	if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		errx(1, "Tried to use framebuffer with incomplete attachment");

	if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
		errx(1, "Tried to use framebuffer with no attachments");

	if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
		errx(1, "OpenGL encountered "
		     "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");

	if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
		errx(1, "OpenGL encountered "
		     "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");

	if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
		errx(1, "Tried to use framebuffer with attachments with "
		     "different number of multisample samples");

	if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
		errx(1, "Tried to use framebuffer with attachments with "
		     "different numbers of layers");
}

/**
 * Set up our colorbuf to be drawn to by OpenGL.
 **/
void
colorbuf_prep(colorbuf_t *colorbuf)
{
	GLenum *buffers;
	size_t i;

	if (colorbuf == current_colorbuf)
		return;

	if (current_colorbuf)
		colorbuf_ungrab(current_colorbuf);

	if (! colorbuf) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDrawBuffer(GL_BACK);

		CHECK_GL;
		return;
	}

	colorbuf_grab(colorbuf);

	if (! colorbuf->num_colorbufs)
		errx(1, "Attempt to use colorbuf with no buffers");

	buffers = xcalloc(colorbuf->num_colorbufs, sizeof(GLenum));

	if (! framebuf_maps) {
		glGenFramebuffers(1, &framebuf);
		glGenRenderbuffers(1, &depth_attach);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_attach);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
				      800, 600);
	}

	if (current_colorbuf == NULL)
		glBindFramebuffer(GL_FRAMEBUFFER, framebuf);

	if (! framebuf_maps) {
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
					  GL_DEPTH_STENCIL_ATTACHMENT,
					  GL_RENDERBUFFER, depth_attach);
	}

	current_colorbuf = colorbuf;

	for (i = 0; i < colorbuf->num_colorbufs;) {
		if (! colorbuf_alloc_framebuffer(colorbuf->colorbufs[i],
						 &buffers[i])) {
			i++;
		} else {
			framebuf_maps_size = 0;
			i = 0;
		}
	}

	glDrawBuffers(colorbuf->num_colorbufs, buffers);
	free(buffers);
	CHECK_GL;
	colorbuf_check_status();
}

/**
 * Copy contents of one buffer to another.
 **/
void
colorbuf_copy(colorbuf_t *in, colorbuf_t *out)
{
	GLuint framebufs[2];
	size_t i;

	if (in == out)
		return;

	glGenFramebuffers(2, framebufs);
	CHECK_GL;

	if (! in) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	} else {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufs[0]);

		for (i = 0; i < in->num_colorbufs; i++)
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
					       GL_COLOR_ATTACHMENT0 + i,
					       GL_TEXTURE_2D,
					       in->colorbufs[i]->map, 0);
	}

	if (! out) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	} else {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufs[1]);

		for (i = 0; i < in->num_colorbufs; i++)
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
					       GL_COLOR_ATTACHMENT0 + i,
					       GL_TEXTURE_2D,
					       out->colorbufs[i]->map, 0);
	}

	glBlitFramebuffer(0,0,800,600,0,0,800,600, GL_COLOR_BUFFER_BIT,
			  GL_NEAREST);
	CHECK_GL;

	if (current_colorbuf) {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuf);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		CHECK_GL;
	}

	glDeleteFramebuffers(2, framebufs);
	CLEAR_GL;
}

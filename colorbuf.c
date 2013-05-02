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
#include <limits.h>
#include <stdint.h>

#include <GL/gl.h>

#include "colorbuf.h"
#include "util.h"

/* Where the hell is this?! */
#ifndef SIZE_T_MAX
#define SIZE_T_MAX (~(size_t)0)
#endif

static colorbuf_t *current_colorbuf = NULL;

static GLuint framebuf;
static texmap_t **framebuf_maps = NULL;
static size_t framebuf_maps_size = 0;

colorbuf_t def_buf = {0};
size_t def_buf_w = 0;
size_t def_buf_h = 0;

/**
 * Update one of the texmaps in framebuf_maps
 **/
void
colorbuf_notify_texmap_change(texmap_t *texmap)
{
	size_t i;

	for (i = 0; i < framebuf_maps_size; i++) {
		if (framebuf_maps[i] != texmap)
			continue;

		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
				       texmap->map, 0);
	}

	CHECK_GL;
}

/**
 * Set the properties of the default buffer.
 **/
void
colorbuf_init_output(unsigned int flags)
{
	if (flags & ~(COLORBUF_VALID_FLAGS))
		errx(1, "Colorbuf initialized with invalid flags");

	flags |= COLORBUF_INITIALIZED;

	if (def_buf.flags)
		errx(1, "Tried to initialize output twice");

	def_buf.flags = flags;
}

/**
 * Set the size of the default buffer.
 **/
void
colorbuf_set_output_geom(size_t w, size_t h)
{
	def_buf_w = w;
	def_buf_h = h;
	glViewport(0, 0, w, h);
	CHECK_GL;
}

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

	if (buf->flags & COLORBUF_AUTO_DEPTH)
		glDeleteRenderbuffers(1, &buf->autodepth);

	free(buf->colorbufs);
	free(buf);
}

/**
 * Create a new colorbuf.
 **/
colorbuf_t *
colorbuf_create(unsigned int flags)
{
	colorbuf_t *ret = xcalloc(1, sizeof(colorbuf_t));

	ret->flags = flags;

	if (flags & ~(COLORBUF_VALID_FLAGS))
		errx(1, "Colorbuf initialized with invalid flags");

	flags |= COLORBUF_INITIALIZED;

	if (flags & COLORBUF_AUTO_DEPTH) {
		glGenRenderbuffers(1, &ret->autodepth);
		CHECK_GL;
	}

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
	if (colorbuf)
		refcount_grab(&colorbuf->refcount);
}

/**
 * Ungrab a colorbuf.
 **/
void
colorbuf_ungrab(colorbuf_t *colorbuf)
{
	if (colorbuf)
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
	if (! buf)
		errx(1, "Cannot append to output framebuffer");

	if (buf->num_colorbufs == colorbuf_max_bufs())
		errx(1, "Platform does not support more than "
		     "%zu color buffers", colorbuf_max_bufs());

	buf->flags &= ~COLORBUF_RENDERBUF_HAS_STORAGE;
	buf->colorbufs = vec_expand(buf->colorbufs, buf->num_colorbufs);
	buf->colorbufs[buf->num_colorbufs++] = texmap;
	texmap_grab(texmap);
	texmap->flags |= TEXMAP_NOTIFY_COLORBUF;

	/* FIXME */
	if (buf == current_colorbuf)
		errx(1, "Haven't implemented append for active colorbuf");

	return buf->num_colorbufs - 1;
}

/**
 * Grab a a colorbuf, inform it that we are a dependency.
 **/
void
colorbuf_dep_grab(colorbuf_t *colorbuf)
{
	if (! colorbuf)
		colorbuf = &def_buf;

	colorbuf_grab(colorbuf);

	colorbuf->deps_total++;
}

/**
 * Ungrab a colorbuf, inform it that we no longer depend on it.
 **/
void
colorbuf_dep_ungrab(colorbuf_t *colorbuf)
{
	if (! colorbuf)
		colorbuf = &def_buf;

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
	if (! colorbuf)
		colorbuf = &def_buf;

	colorbuf->deps_complete++;

	if (colorbuf->deps_complete > colorbuf->deps_total)
		errx(1, "Dependency count mismatch in colorbuf");

	if (colorbuf->deps_complete != colorbuf->deps_total)
		return;

	if (colorbuf->ready_callback)
		colorbuf->ready_callback(colorbuf->ready_callback_data);
}

/**
 * Clear the contents of a colorbuf.
 **/
static void
colorbuf_do_clear(colorbuf_t *in)
{
	GLuint flags = 0;

	if (in->flags & COLORBUF_CLEAR) {
		glClearColor(in->clear_color[0], in->clear_color[1],
			     in->clear_color[2], in->clear_color[3]);
		flags |= GL_COLOR_BUFFER_BIT;
	}

	if (in->flags & COLORBUF_CLEAR_DEPTH) {
		glClearDepth(in->clear_depth);
		flags |= GL_DEPTH_BUFFER_BIT;
	}

	if (in->flags & COLORBUF_CLEAR_STENCIL) {
		glClearStencil(in->clear_stencil);
		flags |= GL_STENCIL_BUFFER_BIT;
	}

	if (flags)
		glClear(flags);
}

/**
 * Notify the colorbuf that its contents are no longer valid.
 **/
void
colorbuf_invalidate(colorbuf_t *colorbuf)
{
	if (! colorbuf)
		colorbuf = &def_buf;

	colorbuf->deps_complete = 0;

	if (colorbuf == current_colorbuf)
		colorbuf_do_clear(colorbuf);
}

/**
 * Any texmap that we put into framebuf_maps needs this destructor so
 * framebuf_maps can be scrubbed. Otherwise if the texmap is freed and then the
 * memory segment is reused as a texmap again we might not know it needs to be
 * added to the framebuf.
 **/
static void
colorbuf_texmap_destruct(void *map)
{
	size_t i;

	for (i = 0; i < framebuf_maps_size; i++)
		if (framebuf_maps[i] == map)
			framebuf_maps[i] = NULL;
}

/**
 * Allocate a color attachment in the framebuffer.
 **/
static int
colorbuf_alloc_framebuffer(texmap_t *map, GLenum *attach)
{
	size_t i;

	for (i = 0; i < framebuf_maps_size && framebuf_maps[i] != map &&
	     framebuf_maps[i]; i++);

	*attach = GL_COLOR_ATTACHMENT0 + i;

	if (i != framebuf_maps_size && framebuf_maps[i] == map)
		return 0;

	if (i == framebuf_maps_size) {
		if (framebuf_maps_size == colorbuf_max_bufs())
			return -1;

		framebuf_maps = vec_expand(framebuf_maps, framebuf_maps_size);
		framebuf_maps_size++;
	}

	framebuf_maps[i] = map;
	refcount_add_destructor_once(&map->refcount, colorbuf_texmap_destruct,
				     map);

	glFramebufferTexture2D(GL_FRAMEBUFFER, *attach, GL_TEXTURE_2D,
			       map->map, 0);
	CHECK_GL;

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
 * Prepare the depth and stencil buffers.
 **/
static void
colorbuf_prep_depth_stencil()
{
	size_t w = SIZE_T_MAX;
	size_t h = SIZE_T_MAX;
	size_t i;
	GLint ifmt = GL_DEPTH_COMPONENT24;
	GLenum attach_type = GL_DEPTH_ATTACHMENT;
	
	if (current_colorbuf->flags & COLORBUF_STENCIL) {
		ifmt = GL_DEPTH24_STENCIL8;
		attach_type = GL_DEPTH_STENCIL_ATTACHMENT;
	} else {
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
					  GL_STENCIL_ATTACHMENT,
					  GL_RENDERBUFFER, 0);
	}

	if (! (current_colorbuf->flags & COLORBUF_AUTO_DEPTH)) {
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
					  GL_DEPTH_STENCIL_ATTACHMENT,
					  GL_RENDERBUFFER, 0);
		CHECK_GL;
		return;
	}

	if (! (current_colorbuf->flags & COLORBUF_RENDERBUF_HAS_STORAGE)) {
		glBindRenderbuffer(GL_RENDERBUFFER,
				   current_colorbuf->autodepth);

		for (i = 0; i < current_colorbuf->num_colorbufs; i++) {
			if (current_colorbuf->colorbufs[i]->w < w)
				w = current_colorbuf->colorbufs[i]->w;
			if (current_colorbuf->colorbufs[i]->h < h)
				h = current_colorbuf->colorbufs[i]->h;
		}

		glRenderbufferStorage(GL_RENDERBUFFER, ifmt, w, h);

		current_colorbuf->flags |= COLORBUF_RENDERBUF_HAS_STORAGE;
	}

	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
				  attach_type,
				  GL_RENDERBUFFER,
				  current_colorbuf->autodepth);
	CHECK_GL;
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
		colorbuf_do_clear(&def_buf);
		return;
	}

	colorbuf_grab(colorbuf);

	if (! colorbuf->num_colorbufs)
		errx(1, "Attempt to use colorbuf with no buffers");

	buffers = xcalloc(colorbuf->num_colorbufs, sizeof(GLenum));

	if (! framebuf_maps)
		glGenFramebuffers(1, &framebuf);

	if (current_colorbuf == NULL)
		glBindFramebuffer(GL_FRAMEBUFFER, framebuf);

	current_colorbuf = colorbuf;
	colorbuf_prep_depth_stencil();

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

	if (colorbuf->deps_complete == 0)
		colorbuf_do_clear(colorbuf);
}

/**
 * Copy contents of one buffer to another.
 **/
void
colorbuf_copy(colorbuf_t *in, colorbuf_t *out)
{
	GLuint framebufs[2];
	size_t i;
	size_t w_in = SIZE_T_MAX;
	size_t h_in = SIZE_T_MAX;
	size_t w_out = SIZE_T_MAX;
	size_t h_out = SIZE_T_MAX;

	if (in == out)
		return;

	glGenFramebuffers(2, framebufs);
	CHECK_GL;

	if (! in) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		w_in = def_buf_w;
		h_in = def_buf_h;
	} else {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufs[0]);

		for (i = 0; i < in->num_colorbufs; i++) {
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
					       GL_COLOR_ATTACHMENT0 + i,
					       GL_TEXTURE_2D,
					       in->colorbufs[i]->map, 0);

			if (in->colorbufs[i]->w < w_in)
				w_in = in->colorbufs[i]->w;
			if (in->colorbufs[i]->h < h_in)
				h_in = in->colorbufs[i]->h;
		}
	}

	if (! out) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		w_out = def_buf_w;
		h_out = def_buf_h;
	} else {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufs[1]);

		for (i = 0; i < in->num_colorbufs; i++) {
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
					       GL_COLOR_ATTACHMENT0 + i,
					       GL_TEXTURE_2D,
					       out->colorbufs[i]->map, 0);

			if (out->colorbufs[i]->w < w_out)
				w_out = out->colorbufs[i]->w;
			if (out->colorbufs[i]->h < h_out)
				h_out = out->colorbufs[i]->h;
		}
	}

	if (!w_in || !w_out || !h_in || !h_out)
		errx(1, "Tried to blit from/to zero-sized colorbuf");

	glBlitFramebuffer(0,0,w_in,h_in,0,0,w_out,h_out, GL_COLOR_BUFFER_BIT,
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

/**
 * Set the color to clear this buffer to.
 **/
void
colorbuf_clear_color(colorbuf_t *in, float color[4])
{
	if (! in)
		in = &def_buf;
	memcpy(in->clear_color, color, 4 * sizeof(float));
}

/**
 * Set the depth to clear this buffer to.
 **/
void
colorbuf_clear_depth(colorbuf_t *in, float depth)
{
	if (! in)
		in = &def_buf;
	in->clear_depth = depth;
}

/**
 * Set the stencil index to clear this buffer to.
 **/
void
colorbuf_clear_stencil(colorbuf_t *in, int index)
{
	if (! in)
		in = &def_buf;
	in->clear_stencil = index;
}

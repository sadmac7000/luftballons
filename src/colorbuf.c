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
#include <limits.h>
#include <stdint.h>

#include <GL/gl.h>

#include "colorbuf.h"
#include "util.h"

static colorbuf_t *current_colorbuf = NULL;

colorbuf_t def_buf = {0};
size_t def_buf_w = 0;
size_t def_buf_h = 0;

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
EXPORT(colorbuf_init_output);

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
EXPORT(colorbuf_set_output_geom);

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

	if (buf->flags & COLORBUF_DEPTH)
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

	flags |= COLORBUF_INITIALIZED | COLORBUF_NEEDS_CLEAR;

	if (flags & COLORBUF_DEPTH)
		glGenRenderbuffers(1, &ret->autodepth);
	else if (flags & COLORBUF_STENCIL)
		errx(1, "COLORBUF_STENCIL requires COLORBUF_DEPTH");

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, colorbuf_destroy, ret);

	glGenFramebuffers(1, &ret->framebuf);
	CHECK_GL;

	return ret;
}
EXPORT(colorbuf_create);

/**
 * Grab a colorbuf.
 **/
void
colorbuf_grab(colorbuf_t *colorbuf)
{
	if (colorbuf)
		refcount_grab(&colorbuf->refcount);
}
EXPORT(colorbuf_grab);

/**
 * Ungrab a colorbuf.
 **/
void
colorbuf_ungrab(colorbuf_t *colorbuf)
{
	if (colorbuf)
		refcount_ungrab(&colorbuf->refcount);
}
EXPORT(colorbuf_ungrab);

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
EXPORT(colorbuf_max_bufs);

/**
 * Set the buffer at the given index in a colorbuf.
 **/
void
colorbuf_set_buf(colorbuf_t *buf, size_t idx, texmap_t *texmap)
{
	size_t i;
	GLuint framebuf_prev = current_colorbuf ? current_colorbuf->framebuf : 0;

	if (! buf)
		errx(1, "Cannot add attachments to output framebuffer");

	if (buf->num_colorbufs == colorbuf_max_bufs())
		errx(1, "Platform does not support more than "
		     "%zu color buffers", colorbuf_max_bufs());

	buf->flags &= ~COLORBUF_RENDERBUF_HAS_STORAGE;

	if (texmap)
		texmap_grab(texmap);

	for (i = 0; i < buf->num_colorbufs; i++)
		if (buf->colorbuf_attach_pos[i] == idx)
			break;

	if (i == buf->num_colorbufs) {
		buf->colorbufs = vec_expand(buf->colorbufs, buf->num_colorbufs);
		buf->colorbuf_attach_pos = vec_expand(buf->colorbuf_attach_pos, buf->num_colorbufs);
		buf->num_colorbufs++;
	} else {
		texmap_ungrab(buf->colorbufs[i]);
	}

	buf->colorbuf_attach_pos[i] = idx;
	buf->colorbufs[i] = texmap;

	if (! texmap) {
		buf->colorbufs = vec_del(buf->colorbufs,
					 buf->num_colorbufs, i);
		buf->colorbuf_attach_pos = vec_del(buf->colorbuf_attach_pos,
						   buf->num_colorbufs, i);
		buf->num_colorbufs--;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, buf->framebuf);

	if (texmap)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
				       GL_TEXTURE_2D, texmap->map, 0);
	else
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
				       GL_TEXTURE_2D, 0, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuf_prev);

	/* FIXME: Set glDrawBuffers if we're the current colorbuf */
}
EXPORT(colorbuf_set_buf);

/**
 * Set a depth buffer texmap for this colorbuf.
 **/
void
colorbuf_set_depth_buf(colorbuf_t *colorbuf, texmap_t *texmap)
{
	if (!texmap && !colorbuf->depth_texmap)
		return;

	colorbuf->flags &= ~COLORBUF_RENDERBUF_HAS_STORAGE;

	if (! (colorbuf->flags & COLORBUF_DEPTH))
		errx(1, "Cannot set a depth buffer texmap for a "
		     "colorbuffer with no depth buffer");

	if (colorbuf->depth_texmap)
		texmap_ungrab(colorbuf->depth_texmap);
	else
		glDeleteRenderbuffers(1, &colorbuf->autodepth);

	colorbuf->depth_texmap = texmap;

	if (! texmap) {
		glGenRenderbuffers(1, &colorbuf->autodepth);
		return;
	}

	texmap_grab(texmap);

	if (! (texmap->flags & TEXMAP_DEPTH))
		errx(1, "Cannot use a non-depth texmap as a depth buffer");

	if (colorbuf->flags & COLORBUF_STENCIL)
		if (! (texmap->flags & TEXMAP_STENCIL))
			errx(1, "Colorbuf with a stencil buffer needs a depth"
			     " texmap that has a stencil buffer");
}
EXPORT(colorbuf_set_depth_buf);

/**
 * Clear the contents of the current colorbuf.
 **/
static void
colorbuf_do_clear(void)
{
	GLuint flags = 0;

	colorbuf_t *in = current_colorbuf ?: &def_buf;

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
colorbuf_clear(colorbuf_t *colorbuf)
{
	if (! colorbuf)
		colorbuf = &def_buf;

	if (colorbuf == current_colorbuf)
		colorbuf_do_clear();
	else
		colorbuf->flags |= COLORBUF_NEEDS_CLEAR;
}
EXPORT(colorbuf_clear);

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
 * Prepare the depth and stencil buffers from a texmap.
 **/
static void
colorbuf_prep_depth_stencil_texmap()
{
	GLenum attach_type = GL_DEPTH_ATTACHMENT;

	if (current_colorbuf->depth_texmap->flags & TEXMAP_STENCIL)
		attach_type = GL_DEPTH_STENCIL_ATTACHMENT;

	glFramebufferTexture2D(GL_FRAMEBUFFER, attach_type,
			       GL_TEXTURE_2D,
			       current_colorbuf->depth_texmap->map, 0);
	CHECK_GL;
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

	if (current_colorbuf->depth_texmap) {
		colorbuf_prep_depth_stencil_texmap();
		return;
	}

	if (current_colorbuf->flags & COLORBUF_STENCIL) {
		ifmt = GL_DEPTH24_STENCIL8;
		attach_type = GL_DEPTH_STENCIL_ATTACHMENT;
	} else {
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
					  GL_STENCIL_ATTACHMENT,
					  GL_RENDERBUFFER, 0);
	}

	if (! (current_colorbuf->flags & COLORBUF_DEPTH)) {
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
 * Set the draw buffers for the current colorbuf.
 **/
static void
colorbuf_set_draw(void)
{
	size_t bufcnt = 0;
	GLenum *buffers;
	size_t i;

	if (! current_colorbuf) {
		glDrawBuffer(GL_BACK);
		return;
	}

	for (i = 0; i < current_colorbuf->num_colorbufs; i++)
		if (current_colorbuf->colorbuf_attach_pos[i] > bufcnt)
			bufcnt = current_colorbuf->colorbuf_attach_pos[i];

	buffers = xcalloc(bufcnt + 1, sizeof(GLenum));

	while (bufcnt--) buffers[bufcnt] = GL_NONE;

	for (i = 0; i < current_colorbuf->num_colorbufs; i++)
		buffers[i] = GL_COLOR_ATTACHMENT0 +
			current_colorbuf->colorbuf_attach_pos[i];

	glDrawBuffers(current_colorbuf->num_colorbufs, buffers);
	free(buffers);
	CHECK_GL;
}
/**
 * Set up our colorbuf to be drawn to by OpenGL.
 **/
void
colorbuf_prep(colorbuf_t *colorbuf)
{
	if (colorbuf == current_colorbuf)
		return;

	if (current_colorbuf)
		colorbuf_ungrab(current_colorbuf);

	if (! colorbuf) {
		current_colorbuf = NULL;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDrawBuffer(GL_BACK);

		colorbuf_set_draw();

		if (def_buf.flags & COLORBUF_NEEDS_CLEAR)
			colorbuf_do_clear();
		def_buf.flags &= ~COLORBUF_NEEDS_CLEAR;

		CHECK_GL;
		return;
	}

	colorbuf_grab(colorbuf);

	if (! colorbuf->num_colorbufs)
		errx(1, "Attempt to use colorbuf with no buffers");

	current_colorbuf = colorbuf;

	glBindFramebuffer(GL_FRAMEBUFFER, colorbuf->framebuf);

	colorbuf_check_status();
	colorbuf_prep_depth_stencil();
	colorbuf_set_draw();

	if (colorbuf->flags & COLORBUF_NEEDS_CLEAR)
		colorbuf_do_clear();

	colorbuf->flags &= ~COLORBUF_NEEDS_CLEAR;
}

/**
 * Copy contents of one buffer to another.
 **/
void
colorbuf_copy(colorbuf_t *in, size_t in_idx, colorbuf_t *out, size_t out_idx)
{
	size_t i;
	size_t w_in = SIZE_T_MAX;
	size_t h_in = SIZE_T_MAX;
	size_t w_out = SIZE_T_MAX;
	size_t h_out = SIZE_T_MAX;

	if (in == out)
		return;

	if (! in) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		w_in = def_buf_w;
		h_in = def_buf_h;
		in = &def_buf;
		glReadBuffer(GL_BACK);
	} else {
		for (i = 0; i < in->num_colorbufs; i++) {
			if (in->colorbuf_attach_pos[i] == in_idx)
				in_idx = i;
			if (in->colorbufs[i]->w < w_in)
				w_in = in->colorbufs[i]->w;
			if (in->colorbufs[i]->h < h_in)
				h_in = in->colorbufs[i]->h;
		}

		if (in_idx == in->num_colorbufs)
			return;

		glBindFramebuffer(GL_READ_FRAMEBUFFER, in->framebuf);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + in_idx);
	}

	if (! out) {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		w_out = def_buf_w;
		h_out = def_buf_h;
		out = &def_buf;
		glDrawBuffer(GL_BACK);
	} else {
		for (i = 0; i < out->num_colorbufs; i++) {
			if (out->colorbuf_attach_pos[i] == out_idx)
				out_idx = i;
			if (out->colorbufs[i]->w < w_out)
				w_out = out->colorbufs[i]->w;
			if (out->colorbufs[i]->h < h_out)
				h_out = out->colorbufs[i]->h;
		}

		if (out_idx == out->num_colorbufs)
			goto reset;

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, out->framebuf);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + out_idx);
	}

	if (!w_in || !w_out || !h_in || !h_out)
		errx(1, "Tried to blit from/to zero-sized colorbuf");

	glBlitFramebuffer(0,0,w_in,h_in,0,0,w_out,h_out,
			  GL_COLOR_BUFFER_BIT, GL_LINEAR);
	CHECK_GL;

reset:
	if (current_colorbuf) {
		glBindFramebuffer(GL_FRAMEBUFFER, current_colorbuf->framebuf);
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	colorbuf_set_draw();
	CHECK_GL;
}
EXPORT(colorbuf_copy);

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
EXPORT(colorbuf_clear_color);

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
EXPORT(colorbuf_clear_depth);

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
EXPORT(colorbuf_clear_stencil);

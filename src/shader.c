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

#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "shader.h"
#include "vbuf.h"
#include "util.h"

static shader_t *current_shader = NULL;

/**
 * Compile a shader from a string containing glsl.
 **/
static GLuint
shader_string(GLenum type, const char *shader_name, const GLchar *data,
	      GLint len)
{
	GLchar *log;
	GLuint shader;
	GLint status;
	GLint log_len;

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &data, &len);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	CHECK_GL;
	if (status != GL_FALSE)
		return shader;

	log = xmalloc(log_len + 1);

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
	glGetShaderInfoLog(shader, log_len, NULL, log);

	errx(1, "Could not compile %s: %s", shader_name, log);
}

/**
 * Load an OpenGL shader from a file.
 **/
static GLuint
shader_file(GLenum type, const char *filename)
{
	int fd = open(filename, O_RDONLY | O_CLOEXEC);
	GLint size;
	GLchar *data;
	GLuint shader;

	if (fd < 0)
		err(1, "Could not open shader file %s", filename);

	size = lseek(fd, 0, SEEK_END);

	if (size < 0)
		err(1, "Could not find end of shader file %s", filename);

	data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);

	if (! data)
		err(1, "Could not map shader file");

	shader = shader_string(type, filename, data, size);

	munmap(data, size);
	return shader;
}

/**
 * Link a shader.
 **/
static void
shader_link(shader_t *shader)
{
	GLint status;
	GLint log_len;
	GLchar *log;

	glLinkProgram(shader->gl_handle);
	glGetProgramiv(shader->gl_handle, GL_LINK_STATUS, &status);

	CHECK_GL;
	if (status != GL_FALSE)
		return;

	log = xmalloc(log_len + 1);

	glGetProgramiv(shader->gl_handle, GL_INFO_LOG_LENGTH, &log_len);
	glGetProgramInfoLog(shader->gl_handle, log_len, NULL, log);

	errx(1, "Could not link shaders: %s", log);
}

/**
 * Instantiate a shader.
 **/
static shader_t *
shader_instantiate(void)
{
	shader_t *ret = xmalloc(sizeof(shader_t));

	ret->gl_handle = glCreateProgram();
	ret->uniforms = NULL;
	ret->uniform_count = 0;
	CHECK_GL;
	return ret;
}

/**
 * Destructor for a shader_t
 **/
static void
shader_destructor(void *shader_)
{
	shader_t *shader = shader_;
	size_t i;

	glDeleteProgram(shader->gl_handle);
	CLEAR_GL;

	for (i = 0; i < shader->uniform_count; i++)
		uniform_ungrab(shader->uniforms[i]);

	free(shader);
}

/**
 * Given a file for a vertex shader and fragment shader, load them.
 **/
shader_t *
shader_create(const char *vertex, const char *frag)
{
	GLuint vert_shader = shader_file(GL_VERTEX_SHADER, vertex);
	GLuint frag_shader = shader_file(GL_FRAGMENT_SHADER, frag);

	shader_t *ret = shader_instantiate();

	glAttachShader(ret->gl_handle, vert_shader);
	glAttachShader(ret->gl_handle, frag_shader);

	shader_link(ret);

	glDetachShader(ret->gl_handle, vert_shader);
	glDetachShader(ret->gl_handle, frag_shader);
	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	CHECK_GL;

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, shader_destructor, ret);
	return ret;
}
EXPORT(shader_create);

/**
 * Grab a shader.
 **/
void
shader_grab(shader_t *shader)
{
	refcount_grab(&shader->refcount);
}
EXPORT(shader_grab);

/**
 * Ungrab a shader
 **/
void
shader_ungrab(shader_t *shader)
{
	refcount_ungrab(&shader->refcount);
}
EXPORT(shader_ungrab);

/**
 * Set vertex attributes.
 **/
void
shader_set_vertex_attrs()
{
	GLint attrs;
	GLint namesz;
	GLint i;
	GLchar *name;
	GLint sz;
	GLenum type;

	if (! current_shader)
		return;

	glGetProgramiv(current_shader->gl_handle, GL_ACTIVE_ATTRIBUTES,
		       &attrs);
	glGetProgramiv(current_shader->gl_handle,
		       GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &namesz);

	name = xmalloc(namesz);

	for (i = 0; i < attrs; i++) {
		glGetActiveAttrib(current_shader->gl_handle, i, namesz,
				  NULL, &sz, &type, name);

		vbuf_setup_vertex_attribute(name, i);
	}

	CHECK_GL;
	free(name);
}

/**
 * Enter this shader into the OpenGL state.
 **/
void
shader_activate(shader_t *shader)
{
	texmap_end_unit_generation();
	if (current_shader == shader)
		return;

	current_shader = shader;
	glUseProgram(shader->gl_handle);
	shader_set_vertex_attrs();
	CHECK_GL;
}
EXPORT(shader_activate);

/**
 * Set a uniform value to a 2D texture.
 **/
static void
shader_set_uniform_samp2D(shader_t *shader, const char *name, texmap_t *map)
{
	GLint loc = glGetUniformLocation(shader->gl_handle, name);
	size_t unit = texmap_get_texture_unit(map);

	glUniform1i(loc, unit);

	glBindSampler(unit, map->sampler);
	CHECK_GL;
}

/**
 * Apply a uniform to a shader, in OpenGL terms.
 **/
static void
shader_apply_uniform(shader_t *shader, uniform_t *uniform)
{
	shader_activate(shader);
	GLint loc = glGetUniformLocation(shader->gl_handle, uniform->name);

	switch (uniform->type) {
		case UNIFORM_MAT4:
			glUniformMatrix4fv(loc, 1, GL_FALSE,
					   uniform->value.data_ptr);
			break;
		case UNIFORM_VEC4:
			glUniform4fv(loc, 1, uniform->value.data_ptr);
			break;
		case UNIFORM_TEXMAP:
			shader_set_uniform_samp2D(shader, uniform->name,
						  uniform->value.data_ptr);
			break;
		case UNIFORM_UINT:
			glUniform1i(loc, (int)uniform->value.uint);
			break;
		default:
			errx(1, "Unreachable statement");
	}

	CHECK_GL;
}

/**
 * Set a uniform value.
 **/
void
shader_set_uniform(shader_t *shader, uniform_t *uniform)
{
	size_t i;

	uniform_grab(uniform);
	shader_apply_uniform(shader, uniform);

	for (i = 0; i < shader->uniform_count; i++) {
		if (strcmp(shader->uniforms[i]->name, uniform->name))
			continue;

		uniform_ungrab(shader->uniforms[i]);
		shader->uniforms[i] = uniform;
		return;
	}

	shader->uniforms = vec_expand(shader->uniforms, shader->uniform_count);
	shader->uniforms[shader->uniform_count++] = uniform;
}
EXPORT(shader_set_uniform);

/**
 * Set an unstored uniform for the current shader.
 **/
void
shader_set_temp_uniform(uniform_t *uniform)
{
	shader_apply_uniform(current_shader, uniform);
}
EXPORT(shader_set_temp_uniform);

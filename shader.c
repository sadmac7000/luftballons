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
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <GL/glut.h>

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
	return ret;
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

	return ret;
}

/**
 * Set vertex attributes.
 **/
static void
shader_set_vertex_attrs(shader_t *shader, vbuf_t *buffer)
{
	GLint attrs;
	GLint namesz;
	GLint i;
	GLchar *name;
	GLint sz;
	GLenum type;

	glGetProgramiv(shader->gl_handle, GL_ACTIVE_ATTRIBUTES, &attrs);
	glGetProgramiv(shader->gl_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,
		       &namesz);

	name = xmalloc(namesz);

	for (i = 0; i < attrs; i++) {
		glGetActiveAttrib(shader->gl_handle, i, namesz,
				  NULL, &sz, &type, name);

		vbuf_setup_vertex_attribute(buffer, name, i);
	}

	free(name);
}

/**
 * Enter this shader into the OpenGL state.
 **/
void
shader_activate(shader_t *shader, vbuf_t *buffer)
{
	if (current_shader != shader) {
		current_shader = shader;
		glUseProgram(shader->gl_handle);
	}

	vbuf_activate(buffer);
	shader_set_vertex_attrs(shader, buffer);
}

/**
 * Set a uniform value to a 4x4 matrix.
 **/
void
shader_set_uniform_mat(shader_t *shader, const char *name, float mat[16])
{
	GLint loc = glGetUniformLocation(shader->gl_handle, name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, mat);
}

/**
 * Set a uniform value to a 4 element vector.
 **/
void
shader_set_uniform_vec(shader_t *shader, const char *name, float vec[4])
{
	GLint loc = glGetUniformLocation(shader->gl_handle, name);
	glUniform4fv(loc, 1, vec);
}

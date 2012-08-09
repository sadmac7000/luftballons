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

/**
 * Load an OpenGL shader from a file.
 **/
static GLuint
shader_file(GLenum type, const char *filename)
{
	int fd = open(filename, O_RDONLY | O_CLOEXEC);
	GLint size;
	const GLchar * data;
	GLchar *log;
	GLuint shader;
	GLint status;
	GLint log_len;

	if (fd < 0)
		err(1, "Could not open shader file %s", filename);

	size = lseek(fd, 0, SEEK_END);

	if (size < 0)
		err(1, "Could not find end of shader file %s", filename);

	data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);

	if (! data)
		err(1, "Could not map shader file");

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &data, &size);
	glCompileShader(shader);
	munmap((void *)data, size);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status != GL_FALSE)
		return shader;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

	log = malloc(log_len + 1);

	if (! log)
		err(1, "Could not allocate memory while "
		    "processing shader error");

	glGetShaderInfoLog(shader, log_len, NULL, log);

	errx(1, "Could not compile %s: %s", filename, log);
}

/**
 * Given a file for a vertex shader and fragment shader, load them.
 **/
GLuint
shader_program(const char *vertex, const char *frag)
{
	GLuint shader_prog;
	GLuint vert_shader = shader_file(GL_VERTEX_SHADER, vertex);
	GLuint frag_shader = shader_file(GL_FRAGMENT_SHADER, frag);
	GLint status;
	GLint log_len;
	GLchar *log;

	shader_prog = glCreateProgram();
	glAttachShader(shader_prog, vert_shader);
	glAttachShader(shader_prog, frag_shader);

	glLinkProgram(shader_prog);
	glGetProgramiv(shader_prog, GL_LINK_STATUS, &status);

	if (status != GL_FALSE) {
		glDetachShader(shader_prog, vert_shader);
		glDetachShader(shader_prog, frag_shader);
		glDeleteShader(vert_shader);
		glDeleteShader(frag_shader);
		return shader_prog;
	}

	glGetProgramiv(shader_prog, GL_INFO_LOG_LENGTH, &log_len);

	log = malloc(log_len + 1);

	if (! log)
		err(1, "Could not allocate memory while processing "
		    "shader link error");

	glGetProgramInfoLog(shader_prog, log_len, NULL, log);

	errx(1, "Could not link shaders: %s", log);
}

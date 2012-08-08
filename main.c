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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <GL/glut.h>

const float verts[] = {
	0.75, 0.75, 0.0, 1.0,
	0.75, -0.75, 0.0, 1.0,
	-0.75, -0.75, 0.0, 1.0,
};

GLuint position_buf = 0;
GLuint shader_prog;

void
load_position_buf(void)
{
	glGenBuffers(1, &position_buf);
	glBindBuffer(GL_ARRAY_BUFFER, position_buf);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), verts,
		     GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint
load_shader_file(GLenum type, const char *filename)
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

void
load_shaders(void)
{
	GLuint vert_shader = load_shader_file(GL_VERTEX_SHADER, "vertex.glsl");
	GLuint frag_shader = load_shader_file(GL_FRAGMENT_SHADER, "fragment.glsl");
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
		return;
	}

	glGetProgramiv(shader_prog, GL_INFO_LOG_LENGTH, &log_len);

	log = malloc(log_len + 1);

	if (! log)
		err(1, "Could not allocate memory while processing "
		    "shader link error");

	glGetProgramInfoLog(shader_prog, log_len, NULL, log);

	errx(1, "Could not link shaders: %s", log);
}

void
render(void)
{
	GLint loc;

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader_prog);

	glBindBuffer(GL_ARRAY_BUFFER, position_buf);

	loc = glGetAttribLocation(shader_prog, "position");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableVertexAttribArray(loc);
	glUseProgram(0);

	glutSwapBuffers();
}

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowPosition(-1,-1);
	glutInitWindowSize(800, 600);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_ACCUM |
			    GLUT_DEPTH | GLUT_STENCIL);

	glutCreateWindow(argv[0]);

	glutDisplayFunc(render);

	load_position_buf();
	load_shaders();
	glutMainLoop();
	return 0;
}

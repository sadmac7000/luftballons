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

#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <GL/glut.h>

#include "shader.h"

const float vert_data[] = {
//coords
	0.0f,    0.5f, -1.0f, 1.0f,
	0.5f, -0.366f, -1.5f, 1.0f,
	-0.5f, -0.366f, -1.0f, 1.0f,
//colors
	1.0f,    0.0f, 0.0f, 1.0f,
	0.0f,    1.0f, 0.0f, 1.0f,
	0.0f,    0.0f, 1.0f, 1.0f,
};

const float scale = 1;
const float near = .5;
const float far = 3.0;

float cam_to_clip_transform[] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1,
	/*
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1.0202, 2.0202,
	0, 0, -1, 0,*/
};

GLuint position_buf = 0;
GLuint shader_prog;

GLsizei win_sz[2] = {800, 600};
int need_reshape = 1;

void
load_position_buf(void)
{
	glGenBuffers(1, &position_buf);
	glBindBuffer(GL_ARRAY_BUFFER, position_buf);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vert_data,
		     GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
handle_reshape(void)
{
	if (! need_reshape)
		return;

	cam_to_clip_transform[0] = scale / (win_sz[0] / (float)win_sz[1]);

	glViewport(0, 0, win_sz[0], win_sz[1]);
}

void
get_offsets(GLfloat *x, GLfloat *y)
{
	const float period = 5000.0;
	const float scale = 3.14159 * 2.0 / period;

	float time = glutGet(GLUT_ELAPSED_TIME);

	float angle = fmodf(time, period) * scale;

	*x = cosf(angle) * 0.5;
	*y = sinf(angle) * 0.5;
}

void
render(void)
{
	GLint vert_pos_loc;
	GLint color_pos_loc;
	GLint offset_loc;
	GLint trans_loc;
	GLfloat x, y;

	handle_reshape();

	get_offsets(&x, &y);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader_prog);

	glBindBuffer(GL_ARRAY_BUFFER, position_buf);

	vert_pos_loc = glGetAttribLocation(shader_prog, "position");
	color_pos_loc = glGetAttribLocation(shader_prog, "colorin");
	offset_loc = glGetUniformLocation(shader_prog, "offset");
	trans_loc = glGetUniformLocation(shader_prog, "transform");

	glUniformMatrix4fv(trans_loc, 1, GL_FALSE, cam_to_clip_transform);
	glUniform4f(offset_loc, x, y, 0.0, 0.0);

	glEnableVertexAttribArray(vert_pos_loc);
	glEnableVertexAttribArray(color_pos_loc);
	glVertexAttribPointer(vert_pos_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(color_pos_loc, 4, GL_FLOAT, GL_FALSE, 0,
			      (void *)(12 * sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableVertexAttribArray(vert_pos_loc);
	glDisableVertexAttribArray(color_pos_loc);
	glUseProgram(0);

	glutSwapBuffers();
	glutPostRedisplay();
}

void
reshape(int x, int y)
{
	win_sz[0] = x;
	win_sz[1] = y;

	need_reshape = 1;
}

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowPosition(-1,-1);
	glutInitWindowSize(win_sz[0], win_sz[1]);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_ACCUM |
			    GLUT_DEPTH | GLUT_STENCIL);

	glutCreateWindow(argv[0]);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glutDisplayFunc(render);
	glutReshapeFunc(reshape);

	cam_to_clip_transform[10] = (near + far)/(near - far);
	cam_to_clip_transform[11] = 2*near*far/(near - far);
	cam_to_clip_transform[14] = -1;
	cam_to_clip_transform[15] = 0;

	cam_to_clip_transform[0] = scale / (win_sz[0] / (float)win_sz[1]);
	cam_to_clip_transform[5] = scale;

	load_position_buf();
	shader_prog = shader_program("vertex.glsl", "fragment.glsl");
	glutMainLoop();
	return 0;
}

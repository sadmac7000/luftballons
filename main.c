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
#include "mesh.h"
#include "buffer.h"

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
};

mesh_t *mesh;
shader_t *shader;

GLsizei win_sz[2] = {800, 600};
int need_reshape = 1;

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
	GLint offset_loc;
	GLint trans_loc;
	GLfloat x, y;

	handle_reshape();

	get_offsets(&x, &y);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	offset_loc = glGetUniformLocation(shader->gl_handle, "offset");
	trans_loc = glGetUniformLocation(shader->gl_handle, "transform");

	glUniformMatrix4fv(trans_loc, 1, GL_FALSE, cam_to_clip_transform);
	glUniform4f(offset_loc, x, y, 0.0, 0.0);

	mesh_draw(mesh);

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
	buffer_t *buffer;
	buf_vdata_t vert_regions[2] = {
		{ "position", 4 * sizeof(float), },
		{ "colorin", 4 * sizeof(float), },
	};

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

	buffer = buffer_create(3, GL_STATIC_DRAW, 2, vert_regions);

	shader = shader_create("vertex.glsl", "fragment.glsl");

	mesh = mesh_create(shader, 3, vert_data, 2, vert_regions);

	if (mesh_add_to_buffer(mesh, buffer))
		errx(1, "Could not add mesh to buffer");

	buffer_ungrab(buffer);

	glutMainLoop();
	return 0;
}

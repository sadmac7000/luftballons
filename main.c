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
#include "vbuf.h"
#include "ebuf.h"

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

const float vert_data_2[] = {
//coords
	0.0f,    0.5f, -1.0f, 1.0f,
	0.5f, -0.366f, -1.0f, 1.0f,
	-0.5f, -0.366f, -1.5f, 1.0f,
//colors
	0.0f,    1.0f, 0.0f, 1.0f,
	1.0f,    0.0f, 0.0f, 1.0f,
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
mesh_t *mesh2;
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
get_offsets(GLfloat *x, GLfloat *y, int direction)
{
	float period = 5000.0;
	float scale = 3.14159 * 2.0 / period;

	float time = glutGet(GLUT_ELAPSED_TIME);

	float angle = fmodf(time, period) * scale;

	if (direction)
		angle = 3.14159 * 2.0 - angle;

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


	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	get_offsets(&x, &y, 0);

	offset_loc = glGetUniformLocation(shader->gl_handle, "offset");
	trans_loc = glGetUniformLocation(shader->gl_handle, "transform");

	glUniformMatrix4fv(trans_loc, 1, GL_FALSE, cam_to_clip_transform);
	glUniform4f(offset_loc, x, y, 0.0, 0.0);

	mesh_draw(mesh);

	get_offsets(&x, &y, 1);

	offset_loc = glGetUniformLocation(shader->gl_handle, "offset");
	trans_loc = glGetUniformLocation(shader->gl_handle, "transform");

	glUniformMatrix4fv(trans_loc, 1, GL_FALSE, cam_to_clip_transform);
	glUniform4f(offset_loc, x, y, 0.0, 0.0);

	mesh_draw(mesh2);

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
	vbuf_t *vbuf;
	ebuf_t *ebuf;
	vbuf_fmt_t vert_regions[2] = {
		{ "position", 4 * sizeof(float), },
		{ "colorin", 4 * sizeof(float), },
	};

	uint16_t elems[] = { 0,1,2 };

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

	vbuf = vbuf_create(6, 2, vert_regions);
	ebuf = ebuf_create(6);

	shader = shader_create("vertex.glsl", "fragment.glsl");

	mesh = mesh_create(shader, 3, vert_data, 3, elems, 2, vert_regions,
			   GL_TRIANGLES);
	mesh2 = mesh_create(shader, 3, vert_data_2, 3, elems, 2, vert_regions,
			    GL_TRIANGLES);

	if (mesh_add_to_vbuf(mesh, vbuf))
		errx(1, "Could not add mesh to vertex buffer");

	if (mesh_add_to_ebuf(mesh, ebuf))
		errx(1, "Could not add mesh to element buffer");

	if (mesh_add_to_vbuf(mesh2, vbuf))
		errx(1, "Could not add mesh to vertex buffer");

	if (mesh_add_to_ebuf(mesh2, ebuf))
		errx(1, "Could not add mesh to element buffer");

	vbuf_ungrab(vbuf);
	ebuf_ungrab(ebuf);

	glutMainLoop();
	return 0;
}

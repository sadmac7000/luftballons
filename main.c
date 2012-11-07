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
#include "object.h"
#include "camera.h"
#include "matrix.h"
#include "quat.h"
#include "draw_queue.h"
#include "dae_load.h"

object_t *cube;
object_t *cube_center;
shader_t *shader;
camera_t *camera;
draw_queue_t *draw_queue;

GLsizei win_sz[2] = {800, 600};
int need_reshape = 1;
float frame_time = 0;

struct {
	int forward;
	int backward;
	int s_left;
	int s_right;
	int t_left;
	int t_right;
	int rise;
	int fall;
	int t_up;
	int t_down;
} movement = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

void
handle_reshape(void)
{
	float aspect = (win_sz[0] / (float)win_sz[1]);

	if (! need_reshape)
		return;
	need_reshape = 0;

	camera_update_aspect(camera, aspect);
	glViewport(0, 0, win_sz[0], win_sz[1]);
}

float
get_angle(float time)
{
	float period = 5000.0;
	float scale = 3.14159 * 2.0 / period;

	float angle = fmodf(time, period) * scale;

	return angle;
}

void
update_camera(float time)
{
	float delta_t = time - frame_time;
	float speed = delta_t * .001;
	float rot_speed = .003;
	float yaw_amt = 0;
	float pitch_amt = 0;
	float dir_vec[3];
	float offset_vec[3] = { 0, 0, 0 };
	float tmp_vec[3];
	float up_vec[3] = { 0, 1, 0 };
	float right_vec[3];
	float new_pos[3];
	float dir_vec_new[3];
	float xz_dist;
	float arcos;

	vec3_subtract(camera->target, camera->pos, dir_vec);
	vec3_normalize(dir_vec, dir_vec);
	vec3_cross(up_vec, dir_vec, right_vec);
	vec3_normalize(right_vec, right_vec);

	if (movement.forward) {
		vec3_scale(dir_vec, tmp_vec, speed);
		vec3_add(tmp_vec, offset_vec, offset_vec);
	}

	if (movement.backward) {
		vec3_scale(dir_vec, tmp_vec, -speed);
		vec3_add(tmp_vec, offset_vec, offset_vec);
	}

	if (movement.s_left) {
		vec3_scale(right_vec, tmp_vec, speed);
		vec3_add(tmp_vec, offset_vec, offset_vec);
	}

	if (movement.s_right) {
		vec3_scale(right_vec, tmp_vec, -speed);
		vec3_add(tmp_vec, offset_vec, offset_vec);
	}

	if (movement.rise) {
		tmp_vec[0] = tmp_vec[2] = 0;
		tmp_vec[1] = speed;
		vec3_add(tmp_vec, offset_vec, offset_vec);
	}

	if (movement.fall) {
		tmp_vec[0] = tmp_vec[2] = 0;
		tmp_vec[1] = -speed;
		vec3_add(tmp_vec, offset_vec, offset_vec);
	}

	if (movement.t_left)
		yaw_amt -= rot_speed;

	if (movement.t_right)
		yaw_amt += rot_speed;

	if (movement.t_up)
		pitch_amt += rot_speed;

	if (movement.t_down)
		pitch_amt -= rot_speed;

	if (yaw_amt) {
		dir_vec_new[1] = dir_vec[1];
		dir_vec_new[0] = dir_vec[0] * cosf(yaw_amt) -
			dir_vec[2] * sinf(yaw_amt);
		dir_vec_new[2] = dir_vec[0] * sinf(yaw_amt) +
			dir_vec[2] * cosf(yaw_amt);

		vec3_dup(dir_vec_new, dir_vec);
	}

	if (pitch_amt) {
		vec3_normalize(dir_vec, dir_vec);
		xz_dist = sqrtf(dir_vec[0] * dir_vec[0] + dir_vec[2] * dir_vec[2]);
		arcos = acosf(xz_dist);
		if (dir_vec[1] < 0)
			arcos = -arcos;
		dir_vec[1] = tanf(arcos + pitch_amt) * xz_dist;
	}


	vec3_add(camera->pos, offset_vec, new_pos);
	camera_move(camera, new_pos, 1);

	if (yaw_amt || pitch_amt) {
		vec3_add(camera->pos, dir_vec, new_pos);
		camera_point(camera, new_pos);
	}
}

void
render(void)
{
	float offset[3] = { -.5, 0, -1.25 };
	float time = glutGet(GLUT_ELAPSED_TIME);
	float angle;
	quat_t cube_rot;
	quat_t center_rot;

	handle_reshape();
	update_camera(time);
	angle = get_angle(time);

	quat_init(&cube_rot, 0, 0, 1, angle);
	quat_init(&center_rot, 0, 0, -1, angle);

	object_set_rotation(cube, &cube_rot);
	object_set_rotation(cube_center, &center_rot);
	object_set_translation(cube, offset);

	draw_queue_draw(draw_queue, cube_center, shader, camera);
	draw_queue_flush(draw_queue);

	glutSwapBuffers();
	glutPostRedisplay();

	frame_time = time;
}

void
reshape(int x, int y)
{
	win_sz[0] = x;
	win_sz[1] = y;

	need_reshape = 1;
}

void
tkey(unsigned char key, int x, int y, int state)
{
	(void)x;
	(void)y;

	if (key == 27)
		exit(0);

	if (key == 'w')
		movement.forward = state;

	if (key == 's')
		movement.backward = state;

	if (key == 'a')
		movement.s_left = state;

	if (key == 'd')
		movement.s_right = state;

	if (key == 'q')
		movement.t_left = state;

	if (key == 'e')
		movement.t_right = state;

	if (key == 'r')
		movement.rise = state;

	if (key == 'f')
		movement.fall = state;

	if (key == 't')
		movement.t_up = state;

	if (key == 'g')
		movement.t_down = state;
}

void
onkey(unsigned char key, int x, int y)
{
	tkey(key, x, y, 1);
}

void
offkey(unsigned char key, int x, int y)
{
	tkey(key, x, y, 0);
}

int
main(int argc, char **argv)
{
	size_t aspect = (win_sz[0] / (float)win_sz[1]);

	size_t dae_mesh_count;
	size_t i;
	object_t **items;

	glutInit(&argc, argv);
	glutInitWindowPosition(-1,-1);
	glutInitWindowSize(win_sz[0], win_sz[1]);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_ACCUM |
			    GLUT_DEPTH | GLUT_STENCIL);

	glutCreateWindow(argv[0]);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onkey);
	glutKeyboardUpFunc(offkey);

	shader = shader_create("vertex.glsl", "fragment.glsl");

	draw_queue = draw_queue_create();

	draw_queue_set_clear(draw_queue, 1, 0.5, 0.0, 0.5, 1.0);
	draw_queue_set_clear_depth(draw_queue, 1);

	cube_center = object_create(NULL, NULL, NULL);

	items = dae_load("ref_model/vcolor_cube_small.dae", &dae_mesh_count);

	if (dae_mesh_count != 1)
		errx(1, "Weird object count %zu loading reference cube",
		     dae_mesh_count);

	cube = items[0];
	free(items);
	object_add_child(cube_center, cube);

	items = dae_load("ref_model/P51_Mustang.dae", &dae_mesh_count);

	for (i = 0; i < dae_mesh_count; i++)
		object_add_child(cube, items[i]);

	free(items);

	camera = camera_create(.01, 3000.0, aspect, 45);

	glutMainLoop();
	return 0;
}

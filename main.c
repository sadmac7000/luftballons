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
#include "texmap.h"
#include "state.h"
#include "colorbuf.h"

object_t *cube;
object_t *cube_center;
camera_t *camera;
draw_queue_t *draw_queue;
state_t *cube_state;
state_t *plane_state;
state_t *canopy_state;
colorbuf_t *cbuf;

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
	CHECK_GL;
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
	float speed = delta_t * .003;
	float rot_speed = .006;
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

	draw_queue_draw(draw_queue, cube_center, camera);

	state_enter(cube_state);
	draw_queue_flush(draw_queue);
	colorbuf_complete_dep(cbuf);

	state_enter(plane_state);
	draw_queue_flush(draw_queue);
	colorbuf_complete_dep(cbuf);

	state_enter(canopy_state);
	draw_queue_flush(draw_queue);
	colorbuf_complete_dep(cbuf);

	colorbuf_copy(cbuf, NULL);
	colorbuf_invalidate(cbuf);

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

void
assign_material(object_t *object)
{
       size_t i;

       if (object->type == OBJ_MESH) {
               if (! strncmp(object->name, "canopy", 6))
                       object->mat_id = 1;
               else
                       object->mat_id = 2;
       }

       for (i = 0; i < object->child_count; i++) {
               assign_material(object->children[i]);
       }
}


int
main(int argc, char **argv)
{
	size_t aspect = (win_sz[0] / (float)win_sz[1]);

	size_t dae_mesh_count;
	size_t i;
	object_t **items;
	texmap_t *plane_map;
	texmap_t *canopy_map;
	shader_t *textured_shader;
	shader_t *vcolor_shader;
	uniform_value_t uvtmp;
	texmap_t *cbuf_texmap;
	float clear_color[4] = { 0.5, 0, 0.5, 1 };

	glutInit(&argc, argv);
	glutInitWindowPosition(-1,-1);
	glutInitWindowSize(win_sz[0], win_sz[1]);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_ACCUM |
			    GLUT_DEPTH | GLUT_STENCIL);

	glutCreateWindow(argv[0]);

	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onkey);
	glutKeyboardUpFunc(offkey);

	cbuf = colorbuf_create();
	colorbuf_set_clear_4f(cbuf, COLORBUF_CLEAR | COLORBUF_CLEAR_DEPTH, clear_color);
	cbuf_texmap = texmap_create(0, 0, 0);
	texmap_init_blank(cbuf_texmap, 0, 800, 600);
	colorbuf_append_buf(cbuf, cbuf_texmap);

	vcolor_shader = shader_create("vertex.glsl", "fragment_vcolor.glsl");
	textured_shader = shader_create("vertex.glsl", "fragment_texmap.glsl");

	cube_state = state_create(vcolor_shader);
	state_set_flags(cube_state, STATE_DEPTH_TEST | STATE_ALPHA_BLEND
			| STATE_BF_CULL);
	cube_state->mat_id = 0;
	state_grab(cube_state);
	state_set_colorbuf(cube_state, cbuf);

	canopy_state = state_create(textured_shader);
	state_set_flags(canopy_state, STATE_DEPTH_TEST | STATE_ALPHA_BLEND
			| STATE_BF_CULL | STATE_TEXTURE_2D);
	canopy_state->mat_id = 1;
	state_grab(canopy_state);
	state_set_colorbuf(canopy_state, cbuf);

	plane_state = state_create(textured_shader);
	state_set_flags(plane_state, STATE_DEPTH_TEST | STATE_ALPHA_BLEND
			| STATE_BF_CULL | STATE_TEXTURE_2D);
	plane_state->mat_id = 2;
	state_grab(plane_state);
	state_set_colorbuf(plane_state, cbuf);

	canopy_map = texmap_create(0, 0, 1);
	plane_map = texmap_create(0, 0, 1);

	texmap_load_image(canopy_map, "ref_model/P51_canopy.tif", 0);
	texmap_set_int_param(canopy_map, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	texmap_set_int_param(canopy_map, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	texmap_set_int_param(canopy_map, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	uvtmp.data_ptr = canopy_map;
	state_set_uniform(canopy_state,
			  uniform_create("diffusemap", UNIFORM_SAMP2D, uvtmp));

	texmap_load_image(plane_map, "ref_model/P51_Mustang.tif", 0);
	texmap_set_int_param(plane_map, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	texmap_set_int_param(plane_map, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	texmap_set_int_param(plane_map, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	uvtmp.data_ptr = plane_map;
	state_set_uniform(plane_state,
			  uniform_create("diffusemap", UNIFORM_SAMP2D, uvtmp));

	draw_queue = draw_queue_create();

	cube_center = object_create(NULL);

	items = dae_load("ref_model/vcolor_cube_small.dae", &dae_mesh_count);

	if (dae_mesh_count != 1)
		errx(1, "Weird object count %zu loading reference cube",
		     dae_mesh_count);

	cube = items[0];
	free(items);
	object_reparent(cube, cube_center);

	items = dae_load("ref_model/P51_Mustang.dae", &dae_mesh_count);

	for (i = 0; i < dae_mesh_count; i++) {
		assign_material(items[i]);
		object_reparent(items[i], cube);
	}

	free(items);

	camera = camera_create(.01, 3000.0, aspect, 45);

	glutMainLoop();
	return 0;
}

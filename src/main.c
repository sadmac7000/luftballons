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

#include <luftballons/shader.h>
#include <luftballons/uniform.h>
#include <luftballons/texmap.h>
#include <luftballons/colorbuf.h>
#include <luftballons/state.h>
#include <luftballons/quat.h>
#include <luftballons/matrix.h>
#include <luftballons/object.h>
#include <luftballons/target.h>
#include <luftballons/dae_load.h>

luft_object_t *cube;
luft_object_t *cube_center;
luft_object_t *camera;
luft_quat_t cam_rot;
luft_state_t *cube_state;
luft_state_t *plane_state;
luft_state_t *canopy_state;
luft_colorbuf_t *cbuf;
luft_texmap_t *cbuf_texmap;
luft_target_t *target;

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

	luft_camera_set_aspect(camera, aspect);
	luft_colorbuf_set_output_geom(win_sz[0], win_sz[1]);
	cbuf_texmap = luft_texmap_create(0, 0, 0);
	luft_texmap_init_blank(cbuf_texmap, 0, win_sz[0], win_sz[1]);
	luft_colorbuf_set_buf(cbuf, 0, cbuf_texmap);
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
	float rot_speed = delta_t * .001;
	float offset_vec[3] = { 0, 0, 0 };
	float pitch_amt = 0;
	float yaw_amt = 0;
	luft_quat_t quat;
	LUFT_MATRIX_DECL_IDENT(translate);
	float rotate[16];

	if (movement.forward)
		offset_vec[2] -= speed;

	if (movement.backward)
		offset_vec[2] += speed;

	if (movement.s_left)
		offset_vec[0] -= speed;

	if (movement.s_right)
		offset_vec[0] += speed;

	if (movement.rise)
		offset_vec[1] += speed;

	if (movement.fall)
		offset_vec[1] -= speed;

	if (movement.t_left)
		yaw_amt += rot_speed;

	if (movement.t_right)
		yaw_amt -= rot_speed;

	if (movement.t_up)
		pitch_amt += rot_speed;

	if (movement.t_down)
		pitch_amt -= rot_speed;

	if (yaw_amt) {
		luft_quat_init(&quat, 0,1,0, yaw_amt);
		luft_quat_mul(&quat, &cam_rot, &cam_rot);
	}

	if (pitch_amt) {
		luft_quat_init(&quat, 1,0,0, pitch_amt);
		luft_quat_mul(&cam_rot, &quat, &cam_rot);
	}

	luft_object_set_rotation(camera, &cam_rot);

	translate[12] = offset_vec[0];
	translate[13] = offset_vec[1];
	translate[14] = offset_vec[2];

	luft_quat_to_matrix(&cam_rot, rotate);

	luft_matrix_multiply(rotate, translate, translate);

	offset_vec[0] = translate[12];
	offset_vec[1] = translate[13];
	offset_vec[2] = translate[14];

	luft_object_move(camera, offset_vec);
}

void
render(void)
{
	float offset[3] = { -.5, 0, -1.25 };
	float time = glutGet(GLUT_ELAPSED_TIME);
	float angle;
	luft_quat_t cube_rot;
	luft_quat_t center_rot;

	handle_reshape();
	update_camera(time);
	angle = get_angle(time);

	luft_quat_init(&cube_rot, 0, 0, 1, angle);
	luft_quat_init(&center_rot, 0, 0, -1, angle);

	luft_object_set_rotation(cube, &cube_rot);
	luft_object_set_rotation(cube_center, &center_rot);
	luft_object_set_translation(cube, offset);

	luft_colorbuf_clear(cbuf);

	luft_target_hit(target);

	luft_colorbuf_copy(cbuf, 0, NULL, 0);

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
assign_material(luft_object_t *object)
{
	luft_object_cursor_t cursor;
	const char *name;

	luft_object_foreach_pre(cursor, object) {
		name = luft_object_get_name(object);

		if (! name)
			continue;

		if (luft_object_get_type(object) == OBJ_MESH) {
			if (! strncmp(name, "canopy", 6))
				luft_object_set_material(object, 1);
			else
				luft_object_set_material(object, 2);
		}
	}

	luft_object_cursor_release(&cursor);
}


int
main(int argc, char **argv)
{
	size_t aspect = (win_sz[0] / (float)win_sz[1]);

	size_t dae_mesh_count;
	size_t i;
	luft_object_t **items;
	luft_texmap_t *plane_map;
	luft_texmap_t *canopy_map;
	luft_shader_t *textured_shader;
	luft_shader_t *vcolor_shader;
	luft_uniform_value_t uvtmp;
	luft_uniform_t *uniform;
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

	luft_colorbuf_init_output(0);

	cbuf = luft_colorbuf_create(LUFT_COLORBUF_CLEAR |
				    LUFT_COLORBUF_CLEAR_DEPTH |
				    LUFT_COLORBUF_DEPTH |
				    LUFT_COLORBUF_STENCIL);
	luft_colorbuf_clear_color(cbuf, clear_color);
	luft_colorbuf_clear_depth(cbuf, 1.0);

	vcolor_shader = luft_shader_create("vertex.glsl", "fragment_vcolor.glsl");
	textured_shader = luft_shader_create("vertex.glsl", "fragment_texmap.glsl");

	cube_state = luft_state_create(vcolor_shader);
	luft_state_set_flags(cube_state, LUFT_STATE_DEPTH_TEST |
			     LUFT_STATE_ALPHA_BLEND | LUFT_STATE_BF_CULL);
	luft_state_set_material(cube_state, 0);
	luft_state_set_colorbuf(cube_state, cbuf);

	canopy_state = luft_state_create(textured_shader);
	luft_state_set_flags(canopy_state, LUFT_STATE_DEPTH_TEST |
			     LUFT_STATE_ALPHA_BLEND | LUFT_STATE_BF_CULL |
			     LUFT_STATE_TEXTURE_2D);
	luft_state_set_material(canopy_state, 1);
	luft_state_set_colorbuf(canopy_state, cbuf);

	plane_state = luft_state_create(textured_shader);
	luft_state_set_flags(plane_state, LUFT_STATE_DEPTH_TEST |
			     LUFT_STATE_ALPHA_BLEND | LUFT_STATE_BF_CULL |
			     LUFT_STATE_TEXTURE_2D);
	luft_state_set_material(plane_state, 2);
	luft_state_set_colorbuf(plane_state, cbuf);

	luft_colorbuf_ungrab(cbuf);

	canopy_map = luft_texmap_create(0, 0, 1);
	plane_map = luft_texmap_create(0, 0, 1);

	luft_texmap_load_image(canopy_map, "../ref_models/P51_canopy.tif", 0);
	luft_texmap_set_int_param(canopy_map, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	luft_texmap_set_int_param(canopy_map, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	luft_texmap_set_int_param(canopy_map, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	uvtmp.data_ptr = canopy_map;
	uniform = luft_uniform_create("diffusemap", UNIFORM_SAMP2D, uvtmp);
	luft_state_set_uniform(canopy_state, uniform);
	luft_uniform_ungrab(uniform);
	luft_texmap_ungrab(canopy_map);

	luft_texmap_load_image(plane_map, "../ref_models/P51_Mustang.tif", 0);
	luft_texmap_set_int_param(plane_map, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	luft_texmap_set_int_param(plane_map, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	luft_texmap_set_int_param(plane_map, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	uvtmp.data_ptr = plane_map;
	uniform = luft_uniform_create("diffusemap", UNIFORM_SAMP2D, uvtmp);
	luft_state_set_uniform(plane_state, uniform);
	luft_uniform_ungrab(uniform);
	luft_texmap_ungrab(plane_map);

	cube_center = luft_object_create(NULL);

	items = luft_dae_load("../ref_models/vcolor_cube_small.dae",
			      &dae_mesh_count);

	if (dae_mesh_count != 1)
		errx(1, "Weird object count %zu loading reference cube",
		     dae_mesh_count);

	cube = items[0];
	free(items);
	luft_object_reparent(cube, cube_center);

	items = luft_dae_load("../ref_models/P51_Mustang.dae",
			      &dae_mesh_count);

	for (i = 0; i < dae_mesh_count; i++) {
		assign_material(items[i]);
		luft_object_reparent(items[i], cube);
	}

	free(items);

	camera = luft_object_create(NULL);
	luft_object_make_camera(camera, 45, .01, 3000.0);
	luft_camera_set_aspect(camera, aspect);
	luft_quat_init(&cam_rot, 0,1,0,0);

	target = luft_target_create(cube_center, camera);
	luft_target_add_state(target, cube_state);
	luft_target_add_state(target, canopy_state);
	luft_target_add_state(target, plane_state);
	luft_state_ungrab(cube_state);
	luft_state_ungrab(canopy_state);
	luft_state_ungrab(plane_state);

	glutMainLoop();
	return 0;
}
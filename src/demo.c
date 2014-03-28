/**
 * Copyright © 2013 Casey Dahlin
 *
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

#include <GLFW/glfw3.h>

#include <luftballons/shader.h>
#include <luftballons/uniform.h>
#include <luftballons/texmap.h>
#include <luftballons/colorbuf.h>
#include <luftballons/quat.h>
#include <luftballons/matrix.h>
#include <luftballons/object.h>
#include <luftballons/draw_proc.h>
#include <luftballons/dae_load.h>

luft_object_t *cube;
luft_object_t *cube_center;
luft_object_t *root;
luft_object_t *camera;
luft_object_t *output_light;
luft_quat_t cam_rot;
luft_colorbuf_t *cbuf_a;
luft_colorbuf_t *cbuf_b;
luft_colorbuf_t *gather_cbuf;
luft_texmap_t *normal_texmap;
luft_texmap_t *diffuse_texmap;
luft_texmap_t *position_texmap;
luft_texmap_t *depth_texmap_a;
luft_texmap_t *depth_texmap_b;
luft_texmap_t *gather_texmap;
luft_draw_proc_t *output_draw_proc;
luft_draw_op_t *cube_op;
luft_draw_op_t *plane_op;
luft_draw_op_t *gather_op;
luft_draw_op_t *output_op;
luft_draw_proc_t *draw_proc_1;
luft_draw_proc_t *draw_proc_2;
luft_draw_proc_t *draw_proc_3;
luft_material_t plane_mat;
luft_material_t canopy_mat;
luft_material_t cube_mat;
luft_material_t light_mat;
GLFWwindow *window;

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

	normal_texmap = luft_texmap_create(0, 0, LUFT_TEXMAP_FLOAT32);
	luft_texmap_init_blank(normal_texmap, 0, win_sz[0], win_sz[1]);
	position_texmap = luft_texmap_create(0, 0, LUFT_TEXMAP_FLOAT32);
	luft_texmap_init_blank(position_texmap, 0, win_sz[0], win_sz[1]);
	diffuse_texmap = luft_texmap_create(0, 0, LUFT_TEXMAP_FLOAT32);
	luft_texmap_init_blank(diffuse_texmap, 0, win_sz[0], win_sz[1]);

	if (depth_texmap_a)
		luft_texmap_ungrab(depth_texmap_a);
	if (depth_texmap_b)
		luft_texmap_ungrab(depth_texmap_b);

	depth_texmap_a = luft_texmap_create(0, 0, LUFT_TEXMAP_DEPTH |
					    LUFT_TEXMAP_STENCIL);
	luft_texmap_init_blank(depth_texmap_a, 0, win_sz[0], win_sz[1]);
	depth_texmap_b = luft_texmap_create(0, 0, LUFT_TEXMAP_DEPTH |
					    LUFT_TEXMAP_STENCIL);
	luft_texmap_init_blank(depth_texmap_b, 0, win_sz[0], win_sz[1]);

	gather_texmap = luft_texmap_create(0, 0, 0);
	luft_texmap_init_blank(gather_texmap, 0, win_sz[0], win_sz[1]);

	luft_colorbuf_set_buf(cbuf_a, 0, normal_texmap);
	luft_colorbuf_set_buf(cbuf_a, 1, position_texmap);
	luft_colorbuf_set_buf(cbuf_a, 2, diffuse_texmap);
	luft_colorbuf_set_depth_buf(cbuf_a, depth_texmap_a);

	luft_colorbuf_set_buf(cbuf_b, 0, normal_texmap);
	luft_colorbuf_set_buf(cbuf_b, 1, position_texmap);
	luft_colorbuf_set_buf(cbuf_b, 2, diffuse_texmap);
	luft_colorbuf_set_depth_buf(cbuf_b, depth_texmap_b);

	luft_colorbuf_set_buf(gather_cbuf, 0, gather_texmap);

	luft_draw_proc_set_uniform(draw_proc_2, LUFT_NO_MATERIAL,
				   LUFT_UNIFORM_TEXMAP, "last_depth",
				   depth_texmap_a);
	luft_draw_proc_set_uniform(draw_proc_3, LUFT_NO_MATERIAL,
				   LUFT_UNIFORM_TEXMAP, "last_depth",
				   depth_texmap_b);

	luft_texmap_ungrab(normal_texmap);
	luft_texmap_ungrab(position_texmap);
	luft_texmap_ungrab(diffuse_texmap);

	luft_draw_op_set_uniform(gather_op, LUFT_NO_MATERIAL,
				 LUFT_UNIFORM_TEXMAP, "normal_buf",
				 normal_texmap);

	luft_draw_op_set_uniform(gather_op, LUFT_NO_MATERIAL,
				 LUFT_UNIFORM_TEXMAP, "position_buf",
				 position_texmap);

	luft_draw_op_set_uniform(gather_op, LUFT_NO_MATERIAL,
				 LUFT_UNIFORM_TEXMAP, "diffuse_buf",
				 diffuse_texmap);

	luft_draw_op_set_uniform(output_op, LUFT_NO_MATERIAL,
				 LUFT_UNIFORM_TEXMAP, "in_buf",
				 gather_texmap);
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
	float rise = 0;
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
		rise += speed;

	if (movement.fall)
		rise -= speed;

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
	offset_vec[1] = translate[13] + rise;
	offset_vec[2] = translate[14];

	luft_object_move(camera, offset_vec);
}

void
render(void)
{
	float offset[3] = { -.5, 0, -1.25 };
	double time = glfwGetTime() * 1000;
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

	luft_draw_proc_run(output_draw_proc);

	glfwSwapBuffers(window);

	frame_time = time;
}

void
reshape(GLFWwindow *window, int x, int y)
{
	(void)window;
	win_sz[0] = x;
	win_sz[1] = y;

	need_reshape = 1;
}

void
tkey(int key, int state)
{
	if (key == GLFW_KEY_ESCAPE)
		exit(0);

	if (key == 'W')
		movement.forward = state;

	if (key == 'S')
		movement.backward = state;

	if (key == 'A')
		movement.s_left = state;

	if (key == 'D')
		movement.s_right = state;

	if (key == 'Q')
		movement.t_left = state;

	if (key == 'E')
		movement.t_right = state;

	if (key == 'R')
		movement.rise = state;

	if (key == 'F')
		movement.fall = state;

	if (key == 'T')
		movement.t_up = state;

	if (key == 'G')
		movement.t_down = state;
}

void
onkey(GLFWwindow *window, int key, int sc, int action, int mods)
{
	(void)window;
	(void)sc;
	(void)mods;

	if (action == GLFW_PRESS) {
		printf("Down\n");
		tkey(key, 1);
	} else if (action == GLFW_RELEASE) {
		printf("Up\n");
		tkey(key, 0);
	} else {
		printf("Rep\n");
	}
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

		if (luft_object_get_type(object) == LUFT_OBJ_MESH) {
			if (! strncmp(name, "canopy", 6))
				luft_object_set_material(object, canopy_mat);
			else
				luft_object_set_material(object, plane_mat);
		} else if (luft_object_get_type(object) == LUFT_OBJ_LIGHT) {
			luft_object_set_material(object, light_mat);
		}
	}

	luft_object_cursor_release(&cursor);
}


void
init_glfw(float clear_color[4])
{
	glfwInit();
	window = glfwCreateWindow(win_sz[0], win_sz[1], "Demo", NULL, NULL);
	glfwMakeContextCurrent(window);

	glfwSetWindowSizeCallback(window, reshape);
	glfwSetKeyCallback(window, onkey);

	luft_colorbuf_init_output(LUFT_COLORBUF_CLEAR_DEPTH |
				  LUFT_COLORBUF_CLEAR |
				  LUFT_COLORBUF_DEPTH |
				  LUFT_COLORBUF_STENCIL);
	luft_colorbuf_clear_color(NULL, clear_color);
	luft_colorbuf_clear_depth(NULL, 1.0);
}

int
main(void)
{
	size_t aspect = (win_sz[0] / (float)win_sz[1]);

	size_t dae_mesh_count;
	size_t i;
	luft_object_t **items;
	luft_texmap_t *plane_map;
	luft_texmap_t *canopy_map;
	luft_shader_t *textured_shader;
	luft_shader_t *vcolor_shader;
	luft_shader_t *gather_shader;
	luft_shader_t *output_shader;
	luft_object_t *light;
	luft_object_t *light_2;
	luft_draw_proc_t *draw_proc_do;
	luft_draw_proc_t *gather_draw_proc_1;
	luft_draw_proc_t *gather_draw_proc_2;
	luft_draw_proc_t *gather_draw_proc_3;
	float clear_color[4] = { 0, 0, 0, 0 };
	float light_color[3] = { 0.0, 0.0, 1.0 };
	float light_offset[4] = { 2.0, 0.0, 0.0, 1.0 };
	float light_color_2[3] = { 1.0, 1.0, 0.0 };
	float light_offset_2[4] = { 0.0, 2.0, 0.0, 1.0 };

	init_glfw(clear_color);
	root = luft_object_create(NULL);

	plane_mat = luft_material_alloc();
	cube_mat = luft_material_alloc();
	canopy_mat = luft_material_alloc();
	light_mat = luft_material_alloc();

	camera = luft_object_create(root);
	luft_object_make_camera(camera, 45, .01, 3000.0);
	luft_camera_set_aspect(camera, aspect);
	luft_quat_init(&cam_rot, 0,1,0,0);

	output_light = luft_object_create(NULL);
	luft_object_make_light(output_light, light_color_2);
	luft_object_set_material(output_light, light_mat);

	cbuf_a = luft_colorbuf_create(LUFT_COLORBUF_CLEAR |
				      LUFT_COLORBUF_CLEAR_DEPTH |
				      LUFT_COLORBUF_DEPTH |
				      LUFT_COLORBUF_STENCIL);
	luft_colorbuf_clear_color(cbuf_a, clear_color);
	luft_colorbuf_clear_depth(cbuf_a, 1.0);

	cbuf_b = luft_colorbuf_create(LUFT_COLORBUF_CLEAR |
				      LUFT_COLORBUF_CLEAR_DEPTH |
				      LUFT_COLORBUF_DEPTH |
				      LUFT_COLORBUF_STENCIL);
	luft_colorbuf_clear_color(cbuf_b, clear_color);
	luft_colorbuf_clear_depth(cbuf_b, 1.0);

	gather_cbuf = luft_colorbuf_create(LUFT_COLORBUF_CLEAR);
	luft_colorbuf_clear_color(gather_cbuf, clear_color);

	vcolor_shader = luft_shader_create("vertex.glsl", "fragment_vcolor.glsl");
	textured_shader = luft_shader_create("vertex.glsl", "fragment_texmap.glsl");
	gather_shader = luft_shader_create("vertex_quad.glsl", "fragment_lighting.glsl");
	output_shader = luft_shader_create("vertex_quad.glsl", "fragment_copy.glsl");

	cube_op = luft_draw_op_create(root, camera);
	luft_draw_op_set_flags(cube_op, LUFT_DEPTH_TEST | LUFT_BF_CULL);
	luft_draw_op_set_blend(cube_op, LUFT_BLEND_NONE);

	plane_op = luft_draw_op_clone(cube_op);

	luft_draw_op_set_shader(plane_op, textured_shader);
	luft_draw_op_set_shader(cube_op, vcolor_shader);
	luft_draw_op_activate_material(cube_op, cube_mat);

	gather_op = luft_draw_op_create(root, camera);
	luft_draw_op_set_shader(gather_op, gather_shader);
	luft_draw_op_set_flags(gather_op, LUFT_BF_CULL);
	luft_draw_op_clear_flags(gather_op, LUFT_DEPTH_TEST);
	luft_draw_op_set_blend(gather_op, LUFT_BLEND_ADDITIVE);
	luft_draw_op_activate_material(gather_op, light_mat);
	luft_draw_op_set_colorbuf(gather_op, gather_cbuf);

	luft_colorbuf_ungrab(gather_cbuf);

	output_op = luft_draw_op_create(output_light, camera);
	luft_draw_op_set_shader(output_op, output_shader);
	luft_draw_op_set_flags(output_op, LUFT_BF_CULL);
	luft_draw_op_clear_flags(output_op, LUFT_DEPTH_TEST);
	luft_draw_op_set_blend(output_op, LUFT_BLEND_REVERSE_ALPHA);
	luft_draw_op_activate_material(output_op, light_mat);

	canopy_map = luft_texmap_create(0, 0, LUFT_TEXMAP_COMPRESSED);
	plane_map = luft_texmap_create(0, 0, LUFT_TEXMAP_COMPRESSED);

	luft_texmap_load_image(canopy_map, "../ref_models/P51_canopy.tif", 0);
	luft_texmap_set_min(canopy_map, LUFT_TEXMAP_INTERP_NEAREST,
			    LUFT_TEXMAP_INTERP_NONE);
	luft_texmap_set_mag(canopy_map, LUFT_TEXMAP_INTERP_NEAREST);
	luft_texmap_set_wrap(canopy_map, LUFT_TEXMAP_WRAP_S, LUFT_TEXMAP_WRAP_CLAMP);

	luft_draw_op_set_uniform(plane_op, canopy_mat, LUFT_UNIFORM_TEXMAP,
				 "diffusemap", canopy_map);
	luft_texmap_ungrab(canopy_map);

	luft_texmap_load_image(plane_map, "../ref_models/P51_Mustang.tif", 0);
	luft_texmap_set_min(plane_map, LUFT_TEXMAP_INTERP_NEAREST,
			    LUFT_TEXMAP_INTERP_NONE);
	luft_texmap_set_mag(plane_map, LUFT_TEXMAP_INTERP_NEAREST);
	luft_texmap_set_wrap(plane_map, LUFT_TEXMAP_WRAP_S, LUFT_TEXMAP_WRAP_CLAMP);

	luft_draw_op_set_uniform(plane_op, plane_mat, LUFT_UNIFORM_TEXMAP,
				 "diffusemap", plane_map);
	luft_texmap_ungrab(plane_map);

	cube_center = luft_object_create(root);

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

	light = luft_object_create(root);
	luft_object_make_light(light, light_color);
	luft_object_move(light, light_offset);
	luft_object_set_material(light, light_mat);

	light_2 = luft_object_create(root);
	luft_object_make_light(light_2, light_color_2);
	luft_object_move(light_2, light_offset_2);
	luft_object_set_material(light_2, light_mat);

	draw_proc_do = luft_draw_proc_create(1);
	luft_draw_proc_clear(draw_proc_do, gather_cbuf);
	luft_draw_proc_draw(draw_proc_do, cube_op);
	luft_draw_proc_draw(draw_proc_do, plane_op);

	draw_proc_1 = luft_draw_proc_create(1);
	luft_draw_proc_set_uniform(draw_proc_1, LUFT_NO_MATERIAL, LUFT_UNIFORM_UINT,
				"last_depth_valid", 0);
	luft_draw_proc_set_colorbuf(draw_proc_1, cbuf_a);

	luft_draw_proc_clear(draw_proc_1, cbuf_a);
	luft_draw_proc_run_other(draw_proc_1, draw_proc_do);

	draw_proc_2 = luft_draw_proc_create(1);
	luft_draw_proc_set_uniform(draw_proc_2, LUFT_NO_MATERIAL, LUFT_UNIFORM_UINT,
				"last_depth_valid", 1);
	luft_draw_proc_set_colorbuf(draw_proc_2, cbuf_b);

	luft_draw_proc_clear(draw_proc_2, cbuf_b);
	luft_draw_proc_run_other(draw_proc_2, draw_proc_do);

	draw_proc_3 = luft_draw_proc_create(1);
	luft_draw_proc_set_uniform(draw_proc_3, LUFT_NO_MATERIAL, LUFT_UNIFORM_UINT,
				"last_depth_valid", 1);
	luft_draw_proc_set_colorbuf(draw_proc_3, cbuf_a);

	luft_draw_proc_clear(draw_proc_3, cbuf_a);
	luft_draw_proc_run_other(draw_proc_3, draw_proc_do);

	luft_draw_proc_ungrab(draw_proc_do);

	gather_draw_proc_1 = luft_draw_proc_create(1);
	luft_draw_proc_run_other(gather_draw_proc_1, draw_proc_1);
	luft_draw_proc_draw(gather_draw_proc_1, gather_op);
	luft_draw_proc_draw(gather_draw_proc_1, output_op);

	gather_draw_proc_2 = luft_draw_proc_create(1);
	luft_draw_proc_run_other(gather_draw_proc_2, draw_proc_2);
	luft_draw_proc_draw(gather_draw_proc_2, gather_op);
	luft_draw_proc_draw(gather_draw_proc_2, output_op);

	gather_draw_proc_3 = luft_draw_proc_create(1);
	luft_draw_proc_run_other(gather_draw_proc_3, draw_proc_3);
	luft_draw_proc_draw(gather_draw_proc_3, gather_op);
	luft_draw_proc_draw(gather_draw_proc_3, output_op);

	output_draw_proc = luft_draw_proc_create(1);
	luft_draw_proc_clear(output_draw_proc, NULL);
	luft_draw_proc_run_other(output_draw_proc, gather_draw_proc_1);
	luft_draw_proc_run_other(output_draw_proc, gather_draw_proc_2);
	luft_draw_proc_run_other(output_draw_proc, gather_draw_proc_3);
	luft_draw_proc_run_other(output_draw_proc, gather_draw_proc_2);

	while (! glfwWindowShouldClose(window)) {
		render();
		glfwPollEvents();
	}
}

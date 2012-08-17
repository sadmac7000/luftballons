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
#include <string.h>

#include <GL/glut.h>

#include "camera.h"
#include "util.h"
#include "matrix.h"

/**
 * Update the transform matrix based on the parameters.
 **/
static void
camera_calc_matrix(camera_t *camera)
{
	float *clip = camera->to_clip_xfrm;
	float scale = camera->zoom;
	float near = camera->near;
	float far = camera->far;
	float aspect = camera->aspect;

	float look_vec[3];
	float up_vec[] = { 0.0, 1.0, 0.0 };
	float right_vec[3];

	float trans_mat[16] = {
		1, 0, 0, -camera->pos[0],
		0, 1, 0, -camera->pos[1],
		0, 0, 1, -camera->pos[2],
		0, 0, 0, 1,
	};

	float rot_mat[16] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};

	clip[0] = scale / aspect;
	clip[5] = scale;
	clip[10] = (near + far)/(near - far);
	clip[11] = 2*near*far/(near - far);
	clip[14] = -1;
	clip[15] = 0;

	vec3_subtract(camera->target, camera->pos, look_vec);
	vec3_normalize(look_vec, look_vec);

	vec3_cross(look_vec, up_vec, right_vec);
	vec3_normalize(right_vec, right_vec);
	vec3_cross(right_vec, look_vec, up_vec);
	vec3_scale(look_vec, look_vec, -1);

	rot_mat[0] = right_vec[0];
	rot_mat[1] = right_vec[1];
	rot_mat[2] = right_vec[2];

	rot_mat[4] = up_vec[0];
	rot_mat[5] = up_vec[1];
	rot_mat[6] = up_vec[2];

	rot_mat[8] = look_vec[0];
	rot_mat[9] = look_vec[1];
	rot_mat[10] = look_vec[2];

	matrix_multiply(rot_mat, trans_mat, camera->to_cam_xfrm);
}

/**
 * Get the final transform for this camera.
 **/
void
camera_get_transform(camera_t *camera, float matrix[16])
{
	matrix_multiply(camera->to_cam_xfrm, camera->to_clip_xfrm, matrix);
}

/**
 * Create a new camera.
 *
 * near: Near clip plane.
 * far: Far clip plane.
 * aspect: Starting aspect ratio.
 **/
camera_t *
camera_create(float near, float far, float aspect)
{
	camera_t *ret = xmalloc(sizeof(camera_t));

	ret->near = near;
	ret->far = far;
	ret->zoom = 1;
	ret->aspect = aspect;
	ret->pos[0] = 0;
	ret->pos[1] = 0;
	ret->pos[2] = 0;
	ret->target[0] = 0;
	ret->target[1] = 0;
	ret->target[2] = -1;

	memset(ret->to_clip_xfrm, 0, 16 * sizeof(float));
	memset(ret->to_cam_xfrm, 0, 16 * sizeof(float));
	camera_calc_matrix(ret);

	return ret;
}

/**
 * Update the camera's aspect ratio.
 **/
void
camera_update_aspect(camera_t *camera, float aspect)
{
	camera->aspect = aspect;
	camera_calc_matrix(camera);
}

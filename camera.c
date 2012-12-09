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
#include <math.h>

#include <GL/gl.h>

#include "camera.h"
#include "util.h"
#include "matrix.h"

/**
 * Update the transform matrix based on the parameters.
 **/
static void
camera_calc_matrix(camera_t *camera)
{
	float scale = camera->zoom * camera->fov_scale;
	float near = camera->near;
	float far = camera->far;
	float aspect = camera->aspect;

	float look_vec[3];
	float up_vec[] = { 0.0, 1.0, 0.0 };
	float right_vec[3];
	MATRIX_DECL_IDENT(rot_mat);

	MATRIX_DECL(trans_mat,
		    1, 0, 0, -camera->pos[0],
		    0, 1, 0, -camera->pos[1],
		    0, 0, 1, -camera->pos[2],
		    0, 0, 0, 1);

	camera->to_clip_xfrm[0] = scale / aspect;
	camera->to_clip_xfrm[1] = 0;
	camera->to_clip_xfrm[2] = 0;
	camera->to_clip_xfrm[3] = 0;
	camera->to_clip_xfrm[4] = 0;
	camera->to_clip_xfrm[5] = scale;
	camera->to_clip_xfrm[6] = 0;
	camera->to_clip_xfrm[7] = 0;
	camera->to_clip_xfrm[8] = 0;
	camera->to_clip_xfrm[9] = 0;
	camera->to_clip_xfrm[10] = (near + far)/(near - far);
	camera->to_clip_xfrm[11] = -1;
	camera->to_clip_xfrm[12] = 0;
	camera->to_clip_xfrm[13] = 0;
	camera->to_clip_xfrm[14] = 2*near*far/(near - far);
	camera->to_clip_xfrm[15] = 0;

	vec3_subtract(camera->target, camera->pos, look_vec);
	vec3_normalize(look_vec, look_vec);

	vec3_cross(look_vec, up_vec, right_vec);
	vec3_normalize(right_vec, right_vec);
	vec3_cross(right_vec, look_vec, up_vec);
	vec3_scale(look_vec, look_vec, -1);

	rot_mat[0] = right_vec[0];
	rot_mat[4] = right_vec[1];
	rot_mat[8] = right_vec[2];

	rot_mat[1] = up_vec[0];
	rot_mat[5] = up_vec[1];
	rot_mat[9] = up_vec[2];

	rot_mat[2] = look_vec[0];
	rot_mat[6] = look_vec[1];
	rot_mat[10] = look_vec[2];

	matrix_multiply(rot_mat, trans_mat, camera->to_cspace_xfrm);
}

/**
 * Calculate FOV scale given an FOV in degrees.
 **/
static float
fov_scale(float fov1x)
{
	/* Convert to double-radians */
	fov1x *= 3.14159;
	fov1x /= 360;

	return cosf(fov1x) / sinf(fov1x);
}

/**
 * Create a new camera.
 *
 * near: Near clip plane.
 * far: Far clip plane.
 * aspect: Starting aspect ratio.
 * fov1x: Field of view at 1x magnification.
 **/
camera_t *
camera_create(float near, float far, float aspect, float fov1x)
{
	camera_t *ret = xmalloc(sizeof(camera_t));

	ret->near = near;
	ret->far = far;
	ret->zoom = 1;
	ret->fov_scale = fov_scale(fov1x);
	ret->aspect = aspect;
	ret->pos[0] = 0;
	ret->pos[1] = 0;
	ret->pos[2] = 0;
	ret->target[0] = 0;
	ret->target[1] = 0;
	ret->target[2] = -1;

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

/**
 * Update the camera's target.
 **/
void
camera_point(camera_t *camera, float target[3])
{
	vec3_dup(target, camera->target);
	camera_calc_matrix(camera);
}

/**
 * Move the camera.
 **/
void
camera_move(camera_t *camera, float pos[3], int track)
{
	if (track) {
		vec3_subtract(camera->target, camera->pos, camera->target);
		vec3_add(camera->target, pos, camera->target);
	}

	vec3_dup(pos, camera->pos);
	camera_calc_matrix(camera);
}

/**
 * Zoom in on the camera's target.
 **/
void
camera_zoom(camera_t *camera, float zoom)
{
	camera->zoom = zoom;
	camera_calc_matrix(camera);
}

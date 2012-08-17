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

#ifndef CAMERA_H
#define CAMERA_H

typedef struct camera {
	float to_clip_xfrm[16];
	float to_cam_xfrm[16];
	float near;
	float far;
	float zoom;
	float aspect;
	float pos[3];
	float target[3];
} camera_t;

#ifdef __cplusplus
extern "C" {
#endif

camera_t *camera_create(float near, float far, float aspect);
void camera_update_aspect(camera_t *camera, float aspect);
void camera_get_transform(camera_t *camera, float matrix[16]);

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_H */



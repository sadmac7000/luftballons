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

#ifndef OBJECT_H
#define OBJECT_H
#include <luftballons/object.h>

#include "mesh.h"
#include "quat.h"
#include "util.h"
#include "refcount.h"

#define object_type_t luft_object_type_t
#define pre_skip_children luft_pre_skip_children

#define OBJ_NODE LUFT_OBJ_NODE
#define OBJ_MESH LUFT_OBJ_MESH
#define OBJ_CAMERA LUFT_OBJ_CAMERA
#define OBJ_LIGHT LUFT_OBJ_LIGHT

/**
 * An object. That is a mesh with a position and render context and all of
 * that.
 *
 * parent: The parent of this object. Object inherits transforms from its parent
 * mat_id: A material ID.
 * name: A name for this object.
 * rot: Amount to rotate this object.
 * trans: Amount to translate this object.
 * scale: Amount to scale this object.
 * pretransform: A transform matrix to apply before our local transforms.
 * transform_cache: Combined transform of this object and its parent.
 * private_transform: Transform to apply to this object, but not its children.
 * children: List of child objects of this object.
 * child_count: Size of the children list.
 * type: What type of object this is.
 * mesh: The mesh to draw at this object's location.
 * camera: A camera to position at this location.
 * refcount: Reference count.
 **/
typedef struct object {
	struct object *parent;
	int mat_id;
	char *name;
	quat_t rot;
	float trans[3];
	float scale[3];
	float pretransform[16];
	float *transform_cache;
	float *private_transform;

	struct object **children;
	size_t child_count;

	object_type_t type;
	union {
		mesh_t *mesh;
		struct camera *camera;
		float light_color[3];
	};

	refcounter_t refcount;
} object_t;

#define object_cursor_t luft_object_cursor_t

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(object_create);
API_DECLARE(object_make_light);
API_DECLARE(object_make_camera);
API_DECLARE(object_set_name);
API_DECLARE(object_grab);
API_DECLARE(object_ungrab);
API_DECLARE(object_rotate);
API_DECLARE(object_move);
API_DECLARE(object_scale);
API_DECLARE(object_set_rotation);
API_DECLARE(object_set_translation);
API_DECLARE(object_set_scale);
API_DECLARE(object_get_transform_mat);
API_DECLARE(object_distance);
API_DECLARE(object_reparent);
API_DECLARE(object_apply_pretransform);
API_DECLARE(object_get_total_transform);
API_DECLARE(object_lookup);
API_DECLARE(object_set_material);

API_DECLARE(camera_set_aspect);

API_DECLARE(object_cursor_init);
API_DECLARE(object_cursor_down);
API_DECLARE(object_cursor_up);
API_DECLARE(object_cursor_start_pre);
API_DECLARE(object_cursor_next_pre);
API_DECLARE(object_cursor_skip_children_pre);
API_DECLARE(object_cursor_release);
API_DECLARE(object_get_name);

object_t *object_get_fs_quad(void);
void object_set_mesh(object_t *object, mesh_t *mesh);

void camera_to_clip(object_t *camera, float mat[16]);
void camera_from_world(object_t *camera, float mat[16]);

/**
 * Iterate objects using a cursor, in a pre-position order.
 **/
#define object_foreach_pre luft_object_foreach_pre

#ifdef __cplusplus
}
#endif

#endif /* OBJECT_H */

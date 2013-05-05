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

#ifndef OBJECT_H
#define OBJECT_H

#include "mesh.h"
#include "quat.h"
#include "shader.h"
#include "camera.h"
#include "refcount.h"

/**
 * Types of objects we place in the scene.
 **/
typedef enum {
	OBJ_NODE,
	OBJ_MESH,
	OBJ_CAMERA,
	OBJ_LIGHT,
} object_type_t;

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

	struct object **children;
	size_t child_count;

	object_type_t type;
	union {
		mesh_t *mesh;
		camera_t *camera;
		float light_color[3];
	};

	refcounter_t refcount;
} object_t;

/**
 * A cursor to iterate through a tree of objects.
 **/
typedef struct object_cursor {
	struct object *current;
	size_t *stack;
	size_t stack_size;
} object_cursor_t;

#ifdef __cplusplus
extern "C" {
#endif

object_t *object_create(object_t *parent);
void object_set_mesh(object_t *object, mesh_t *mesh);
void object_make_light(object_t *object, float color[3]);
void object_set_name(object_t *object, const char *name);
void object_grab(object_t *object);
void object_ungrab(object_t *object);
void object_rotate(object_t *object, quat_t *quat);
void object_move(object_t *object, float vec[3]);
void object_scale(object_t *object, float scale[3]);
void object_set_rotation(object_t *object, quat_t *quat);
void object_set_translation(object_t *object, float vec[3]);
void object_set_scale(object_t *object, float scale[3]);
void object_get_transform_mat(object_t *object, float matrix[16]);
void object_reparent(object_t *object, object_t *parent);
void object_apply_pretransform(object_t *object, float matrix[16]);
void object_get_total_transform(object_t *object, float mat[16]);
object_t *object_lookup(object_t *object, const char *name);

void object_cursor_init(object_cursor_t *cursor, object_t *root);
void object_cursor_down(object_cursor_t *cursor, size_t child);
ssize_t object_cursor_up(object_cursor_t *cursor);
object_t *object_cursor_start_pre(object_cursor_t *cursor, object_t *root);
object_t *object_cursor_next_pre(object_cursor_t *cursor);

/**
 * Iterate objects using a cursor, in a pre-position order.
 **/
#define object_foreach_pre(cursor, obj) \
	for ((obj) = object_cursor_start_pre(&(cursor), (obj)); (obj); \
	     (obj) = object_cursor_next_pre(&(cursor)))

/**
 * Release an object cursor's internal data.
 **/
static inline void
object_cursor_release(object_cursor_t *cursor)
{
	free(cursor->stack);
}

#ifdef __cplusplus
}
#endif

#endif /* OBJECT_H */




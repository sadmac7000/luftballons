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

#ifndef LUFTBALLONS_OBJECT_H
#define LUFTBALLONS_OBJECT_H

#include <luftballons/quat.h>
#include <luftballons/material.h>

#include <stdlib.h>

/**
 * Types of objects we place in the scene.
 **/
typedef enum {
	LUFT_OBJ_NODE,
	LUFT_OBJ_MESH,
	LUFT_OBJ_CAMERA,
	LUFT_OBJ_LIGHT,
} luft_object_type_t;

/**
 * A cursor to iterate through a tree of objects.
 **/
typedef struct object_cursor {
	struct object *current;
	size_t *stack;
	size_t stack_size;
} luft_object_cursor_t;

typedef struct object luft_object_t; 

#ifdef __cplusplus
extern "C" {
#endif

luft_object_t *luft_object_create(luft_object_t *parent);
void luft_object_make_light(luft_object_t *object, float color[3]);
void luft_object_make_camera(luft_object_t *object, float fov,
			     float near, float far);
luft_object_type_t luft_object_get_type(luft_object_t *object);
void luft_object_set_name(luft_object_t *object, const char *name);
const char *luft_object_get_name(luft_object_t *object);
void luft_object_grab(luft_object_t *object);
void luft_object_ungrab(luft_object_t *object);
void luft_object_rotate(luft_object_t *object, luft_quat_t *quat);
void luft_object_move(luft_object_t *object, float vec[3]);
void luft_object_scale(luft_object_t *object, float scale[3]);
void luft_object_set_rotation(luft_object_t *object, luft_quat_t *quat);
void luft_object_set_translation(luft_object_t *object, float vec[3]);
void luft_object_set_scale(luft_object_t *object, float scale[3]);
void luft_object_get_transform_mat(luft_object_t *object, float matrix[16]);
float luft_object_distance(luft_object_t *a, luft_object_t *b);
void luft_object_reparent(luft_object_t *object, luft_object_t *parent);
void luft_object_apply_pretransform(luft_object_t *object, float matrix[16]);
void luft_object_get_total_transform(luft_object_t *object, float mat[16]);
luft_object_t *luft_object_lookup(luft_object_t *object, const char *name);
void luft_object_set_material(luft_object_t *object, luft_material_t mat);
void luft_object_set_draw_distance_local(luft_object_t *object, float dist);
void luft_object_set_draw_distance_children(luft_object_t *object, float dist);
void luft_object_set_draw_distance(luft_object_t *object, float dist);
void luft_object_set_meta(luft_object_t *object, void *meta,
			  void (*meta_destructor)(void *));

void luft_camera_set_aspect(luft_object_t *camera, float aspect);

void luft_object_cursor_init(luft_object_cursor_t *cursor,
			     luft_object_t *root);
void luft_object_cursor_down(luft_object_cursor_t *cursor, size_t child);
ssize_t luft_object_cursor_up(luft_object_cursor_t *cursor);
luft_object_t *luft_object_cursor_start_pre(luft_object_cursor_t *cursor,
					    luft_object_t *root);
luft_object_t *luft_object_cursor_next_pre(luft_object_cursor_t *cursor);
void luft_object_cursor_skip_children_pre(luft_object_cursor_t *cursor);
void luft_object_cursor_release(luft_object_cursor_t *cursor);

/**
 * Iterate objects using a cursor, in a pre-position order.
 **/
#define luft_object_foreach_pre(cursor, obj) \
	for ((obj) = luft_object_cursor_start_pre(&(cursor), (obj)); (obj); \
	     (obj) = luft_object_cursor_next_pre(&(cursor)))

#define luft_pre_skip_children(cursor) ({ \
	luft_object_cursor_skip_children_pre(cursor); \
	continue; \
})

#ifdef __cplusplus
}
#endif

#endif /* LUFTBALLONS_OBJECT_H */


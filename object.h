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

/**
 * A callback for setting up a shader before an object is rendered.
 **/
typedef void (*object_prerender_t)(shader_t *shader);

/**
 * An object. That is a mesh with a position and render context and all of
 * that.
 **/
typedef struct object {
	struct object *parent;
	mesh_t *mesh;
	quat_t rot;
	float trans[3];

	struct object **children;
	size_t child_count;
} object_t;

#ifdef __cplusplus
extern "C" {
#endif

object_t *object_create(mesh_t *mesh);
void object_rotate(object_t *object, quat_t *quat);
void object_move(object_t *object, float vec[3]);
void object_set_rotation(object_t *object, quat_t *quat);
void object_set_translation(object_t *object, float vec[3]);
void object_draw(object_t *object);
void object_get_transform_mat(object_t *object, float matrix[16]);

#ifdef __cplusplus
}
#endif

#endif /* OBJECT_H */




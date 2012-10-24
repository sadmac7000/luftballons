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

#ifndef DRAWLIST_H
#define DRAWLIST_H

#include "util.h"
#include "object.h"

/**
 * A list of objects to draw, all at once. Objects can be added to the list and
 * then told to draw. Buffers are allocated automatically.
 *
 * vbufs: The vertex buffers we can assign to our objects.
 * ebufs: The element buffers we can assign to our objects.
 * objects: Objects to be drawn.
 * object_count: Number of objects to be drawn;
 **/
typedef struct drawlist {
	list_head_t vbufs;
	list_head_t ebufs;
	list_head_t objects;
	size_t object_count;
} drawlist_t;

#ifdef __cplusplus
extern "C" {
#endif

drawlist_t *drawlist_create(void);
void drawlist_add_object(drawlist_t *drawlist, object_t *object);
void drawlist_remove_object(drawlist_t *drawlist, object_t *object);
void drawlist_draw(drawlist_t *drawlist, shader_t *shader, camera_t *camera);

#ifdef __cplusplus
}
#endif

#endif /* DRAWLIST_H */

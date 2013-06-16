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

#ifndef DRAW_QUEUE_H
#define DRAW_QUEUE_H

#include "bufpool.h"
#include "mesh.h"
#include "util.h"
#include "object.h"
#include "shader.h"

#ifdef __cplusplus
extern "C" {
#endif

void draw_queue_draw(object_t *object, object_t *camera);

#ifdef __cplusplus
}
#endif

#endif /* DRAW_QUEUE_H */

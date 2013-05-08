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

#ifndef UNIFORM_H
#define UNIFORM_H
#include <luftballons/uniform.h>

#include <GL/gl.h>

#include "refcount.h"
#include "util.h"

typedef luft_uniform_type_t uniform_type_t;
typedef luft_uniform_value_t uniform_value_t;

/**
 * A uniform.
 **/
typedef struct uniform {
	char *name;
	uniform_type_t type;
	uniform_value_t value;
	refcounter_t refcount;
} uniform_t;

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(uniform_create);
API_DECLARE(uniform_grab);
API_DECLARE(uniform_ungrab);

#ifdef __cplusplus
}
#endif

#endif /* UNIFORM_H */

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

#ifndef LUFT_MATERIAL_H
#define LUFT_MATERIAL_H

#include <stdlib.h>

#ifndef SIZE_T_MAX
#define SIZE_T_MAX (~(size_t)0)
#endif

/**
 * A material ID.
 **/
typedef size_t luft_material_t;

/**
 * The "NULL" material ID.
 **/
static const luft_material_t LUFT_NO_MATERIAL = SIZE_T_MAX;

#ifdef __cplusplus
extern "C" {
#endif

luft_material_t luft_material_alloc(void);
void luft_material_destroy(luft_material_t mat);
int luft_material_is_allocd(luft_material_t mat);

#ifdef __cplusplus
}
#endif

#endif /* LUFT_MATERIAL_H */


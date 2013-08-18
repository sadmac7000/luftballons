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

#ifndef MATERIAL_H
#define MATERIAL_H
#include <luftballons/material.h>

#include "util.h"

#define NO_MATERIAL LUFT_NO_MATERIAL
typedef luft_material_t material_t;

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(material_alloc);
API_DECLARE(material_destroy);
API_DECLARE(material_is_allocd);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL_H */


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

#ifndef QUAT_H
#define QUAT_H
#include <luftballons/quat.h>

#include "util.h"

typedef luft_quat_t quat_t;

#ifdef __cplusplus
extern "C" {
#endif

API_DECLARE(quat_init);
API_DECLARE(quat_init_euler);
API_DECLARE(quat_mul);
API_DECLARE(quat_dup);
API_DECLARE(quat_to_matrix);

#ifdef __cplusplus
}
#endif

#endif /* QUAT_H */



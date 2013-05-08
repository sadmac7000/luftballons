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

#ifndef DAE_LOAD_H
#define DAE_LOAD_H
#include <luftballons/object.h>

#ifdef __cplusplus
extern "C" {
#endif

luft_object_t **luft_dae_load(const char *filename, size_t *count);

#ifdef __cplusplus
}
#endif

#endif /* DAE_LOAD_H */

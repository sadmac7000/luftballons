/**
 * This file is part of Luftballons.
 *
 * Luftballons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Luftballons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Luftballons.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef LUFTBALLONS_STATE_H
#define LUFTBALLONS_STATE_H

/* Enable depth testing in this state */
#define LUFT_DEPTH_TEST		0x1
/* Enable backface culling in this state */
#define LUFT_BF_CULL		0x2

typedef enum {
	LUFT_BLEND_DONTCARE,
	LUFT_BLEND_NONE,
	LUFT_BLEND_ALPHA,
	LUFT_BLEND_REVERSE_ALPHA,
	LUFT_BLEND_ADDITIVE,
} luft_blend_mode_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* LUFTBALLONS_STATE_H */

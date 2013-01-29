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

#ifndef STATE_H
#define STATE_H

/* Enable depth testing in this state */
#define STATE_DEPTH_TEST	0x1
/* Enable alpha blending in this state */
#define STATE_ALPHA_BLEND	0x2
/* Enable backface culling in this state */
#define STATE_BF_CULL		0x4
/* Enable the 2D texture buffer */
#define STATE_TEXTURE_2D	0x8

/**
 * A bin of current misc. OpenGL state.
 *
 * flags: Flags indicating properties of this draw state.
 **/
typedef struct draw_state {
	uint64_t flags;
} state_t;

#ifdef __cplusplus
extern "C" {
#endif

state_t *state_create(void);
void state_enter(state_t *state);

static inline void
state_set_flags(state_t *state, uint64_t flags)
{
	state->flags |= flags;
}

static inline void
state_clear_flags(state_t *state, uint64_t flags)
{
	state->flags &= ~flags;
}

#ifdef __cplusplus
}
#endif

#endif /* STATE_H */

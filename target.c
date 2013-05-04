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

#include "target.h"
#include "util.h"
#include "draw_queue.h"

draw_queue_t *draw_queue;

/**
 * The destructor for a target_t
 **/
void
target_destructor(void *target_)
{
	target_t *target = target_;
	size_t i;

	for (i = 0; i < target->num_states; i++)
		state_ungrab(target->states[i]);
	for (i = 0; i < target->num_deps; i++)
		target_ungrab(target->deps[i]);

	free(target->states);
	free(target->deps);
	/*object_ungrab(target->root);*/
	free(target);
}

/**
 * Create a new target.
 **/
target_t *
target_create(object_t *root, camera_t *camera)
{
	target_t *ret = xcalloc(1, sizeof(target_t));

	ret->root = root;
	ret->camera = camera;
	/*object_grab(root);*/

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, target_destructor, ret);

	return ret;
}

/**
 * Grab a target.
 **/
void
target_grab(target_t *target)
{
	refcount_grab(&target->refcount);
}

/**
 * Ungrab a target.
 **/
void
target_ungrab(target_t *target)
{
	refcount_ungrab(&target->refcount);
}

/**
 * Add a dependency to a target.
 **/
void
target_add_dep(target_t *target, target_t *dep)
{
	target->deps = vec_expand(target->deps, target->num_deps);
	target->deps[target->num_deps++] = dep;
	target_grab(dep);
}

/**
 * Add a state that is passed through in order to hit a target.
 **/
void
target_add_state(target_t *target, state_t *state)
{
	target->states = vec_expand(target->states, target->num_states);
	target->states[target->num_states++] = state;
	state_grab(state);
}

/**
 * Return whether a target is in an array of targets.
 **/
static int
target_in_list(target_t *target, target_t **list, size_t len)
{
	while (len--)
		if (list[len] == target)
			return 1;
	return 0;
}

/**
 * Hit all targets in a list. Assume their dependencies are satisfied.
 **/
static void
target_hit_all_nodep(target_t **targets, size_t num_targets)
{
	size_t i;
	size_t j;

	for (i = 0; i < num_targets; i++) {
		draw_queue_draw(draw_queue, targets[i]->root, targets[i]->camera);

		for (j = 0; j < targets[i]->num_states; j++) {
			state_enter(targets[i]->states[j]);
			draw_queue_flush(draw_queue);
		}
	}
}

/**
 * Hit all targets in a queue of interdependent targets, in order.
 **/
static void
target_drain_queue(target_t **queue, size_t queue_len)
{
	target_t **ready = NULL;
	size_t num_ready = 0;
	size_t i;
	size_t j;

	while (queue_len) {
		for (i = 0; i < queue_len; i++) {
			for (j = 0; j < queue[i]->num_deps; j++)
				if (target_in_list(queue[i]->deps[j], queue,
						   queue_len))
					break;

			if (j < queue[i]->num_deps)
				continue;

			ready = vec_expand(ready, num_ready);
			ready[num_ready++] = queue[i];
		}

		target_hit_all_nodep(ready, num_ready);

		j = 0;
		for (i = 0; i < queue_len; i++) {
			queue[j] = queue[i];

			if (! target_in_list(queue[j], ready, num_ready))
				j++;
		}

		queue_len = j;
		num_ready = 0;
	}
}

/**
 * Hit this target.
 **/
void
target_hit(target_t *target)
{
	target_t **queue = xcalloc(1, sizeof(target_t *));
	size_t queue_len = 1;
	size_t i;
	size_t j;

	queue[0] = target;

	if (! draw_queue)
		draw_queue = draw_queue_create();

	/* Ok, I try to stick to simple arrays instead of interesting data
	 * structures on account of most data sets are small and the low-level
	 * performance is more significant than the time complexity, but
	 * this... there's a distinct chance we've become ridiculous here.
	 * So... FIXME.
	 */
	for (i = 0; i < queue_len; i++) {
		for (j = 0; j < queue[i]->num_deps; j++) {
			if (target_in_list(queue[i]->deps[j], queue,
					   queue_len))
				break;

			queue = vec_expand(queue, queue_len);
			queue[queue_len++] = queue[i]->deps[j];
		}
	}

	target_drain_queue(queue, queue_len);
	free(queue);
}

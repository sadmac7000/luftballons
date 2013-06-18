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

#include "target.h"
#include "util.h"
#include "draw_op.h"

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
	for (i = 0; i < target->num_clear_bufs; i++)
		colorbuf_ungrab(target->clear_bufs[i]);

	for (i = 0; i < target->num_steps; i++) {
		if (target->steps[i].type == TARGET_STEP_DRAW)
			draw_op_ungrab(target->steps[i].draw_op);
		else if (target->steps[i].type == TARGET_STEP_TARGET)
			target_ungrab(target->steps[i].target);
		else
			errx(1, "Encountered target step with unknown type");
	}

	if (target->base_state)
		state_ungrab(target->base_state);

	free(target->states);
	free(target->deps);
	free(target->clear_bufs);
	free(target->steps);
	object_ungrab(target->camera);
	free(target);
}

/**
 * Create a new target.
 *
 * camera: The camera to use while hitting this target.
 * base: The base state for this target.
 * repeat: Repeat count for this target.
 **/
target_t *
target_create(object_t *camera, state_t *base, size_t repeat)
{
	target_t *ret = xcalloc(1, sizeof(target_t));

	ret->camera = camera;
	object_grab(camera);

	ret->base_state = base;

	if (base)
		state_grab(base);

	ret->repeat = repeat;

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, target_destructor, ret);

	return ret;
}
EXPORT(target_create);

/**
 * Grab a target.
 **/
void
target_grab(target_t *target)
{
	refcount_grab(&target->refcount);
}
EXPORT(target_grab);

/**
 * Ungrab a target.
 **/
void
target_ungrab(target_t *target)
{
	refcount_ungrab(&target->refcount);
}
EXPORT(target_ungrab);

/**
 * Add a draw op to our list of steps.
 **/
void
target_draw(target_t *target, draw_op_t *op)
{
	target->steps = vec_expand(target->steps, target->num_steps);

	target->steps[target->num_steps].type = TARGET_STEP_DRAW;
	target->steps[target->num_steps].draw_op = op;
	target->num_steps++;

	draw_op_grab(op);
}
EXPORT(target_draw);

/**
 * Add another target to our list of steps.
 **/
void
target_hit_other(target_t *target, target_t *other)
{
	target->steps = vec_expand(target->steps, target->num_steps);

	target->steps[target->num_steps].type = TARGET_STEP_TARGET;
	target->steps[target->num_steps].target = other;
	target->num_steps++;

	target_grab(other);
}
EXPORT(target_hit_other);

/**
 * Set a colorbuf to be cleared by this target after its dependencies are
 * satisfied, but before it runs its draw operations.
 **/
void
target_clear_buf(target_t *target, colorbuf_t *buf)
{
	target->clear_bufs = vec_expand(target->clear_bufs,
					target->num_clear_bufs);

	target->clear_bufs[target->num_clear_bufs++] = buf;
	colorbuf_grab(buf);
}
EXPORT(target_clear_buf);

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
 * Add a dependency to a target.
 **/
void
target_add_dep(target_t *target, target_t *dep)
{
	if (target_in_list(dep, target->deps, target->num_deps))
		errx(1, "A target cannot depend on a target twice");

	target_grab(dep);

	target->deps = vec_expand(target->deps, target->num_deps);
	target->deps[target->num_deps++] = dep;
}
EXPORT(target_add_dep);

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
EXPORT(target_add_state);

/**
 * Perform one step of a target.
 **/
static void
target_do_step(target_step_t *step)
{
	if (step->type == TARGET_STEP_DRAW)
		draw_op_exec(step->draw_op);
	if (step->type == TARGET_STEP_TARGET)
		target_hit(step->target);
	else
		errx(1, "Encountered target step with unknown type");
}

/**
 * Hit all targets in a list. Assume their non-sequential
 * dependencies are satisfied.
 **/
static void
target_hit_all_nodep(target_t **targets, size_t num_targets)
{
	size_t i;
	size_t j;
	size_t k;

	for (i = 0; i < num_targets; i++) {
		for (k = 0; k < targets[i]->num_clear_bufs; k++)
			colorbuf_clear(targets[i]->clear_bufs[k]);

		if (targets[i]->base_state)
			state_push(targets[i]->base_state);

		for (k = 0; k < targets[i]->num_steps; k++)
			target_do_step(&targets[i]->steps[k]);

		for (j = 0; j < targets[i]->num_states; j++) {
			draw_op_t *op = draw_op_create(targets[i]->states[j]->root,
						       targets[i]->camera,
						       targets[i]->states[j]);
			if (! targets[i]->states[j]->root)
				return;

			draw_op_exec(op);
			draw_op_ungrab(op);
		}

		if (targets[i]->base_state)
			state_pop(targets[i]->base_state);
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
 * Hit this target exactly once.
 **/
static void
target_hit_once(target_t *target)
{
	target_t **queue = NULL;
	size_t queue_len = 1;
	size_t i;
	size_t j;

	queue = vec_expand(queue, 0);
	queue[0] = target;

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

/**
 * Hit this target
 **/
void
target_hit(target_t *target)
{
	size_t count = target->repeat;
	size_t i;

	for (i = 0; i < count; i++)
		target_hit_once(target);
}
EXPORT(target_hit);

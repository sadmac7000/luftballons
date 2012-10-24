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

#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <string.h>
#include <err.h>

#define OFFSET_OF(type, member) ((uintptr_t)&((type *)0)->member)
#define CONTAINER_OF(ptr, type, member) ((type *)(((char *)ptr) - \
						  OFFSET_OF(type, member)))

#ifdef __cplusplus
extern "C" {
#endif

static inline void *
xmalloc(size_t size)
{
	void *ret = malloc(size);

	if (!ret)
		errx(1, "Could not allocate memory");

	return ret;
}

static inline void *
xrealloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);

	if (!ret)
		errx(1, "Could not allocate memory");

	return ret;
}

static inline void *
xcalloc(size_t nmemb, size_t size)
{
	void *ret = calloc(nmemb, size);

	if (!ret)
		errx(1, "Could not allocate memory");

	return ret;
}

static inline char *
xstrdup(const char *str)
{
	char *ret = strdup(str);

	if (!ret)
		errx(1, "Could not allocate memory");

	return ret;
}

typedef struct list {
	struct list *next, *prev;
} list_node_t;

typedef list_node_t list_head_t;

#define foreach(head_) \
	for (list_node_t *head = (head_), *pos = head->next; pos != head; \
	     pos = pos->next)

/**
 * Check to see if a list is empty, or if a node is not in a list.
 */
static inline int
list_empty(list_node_t *node)
{
	return node->next == node;
}

/**
 * Initialize a list node or head.
 **/
static inline void
list_init(list_node_t *node)
{
	node->next = node;
	node->prev = node;
}

/**
 * Insert an element into a list after the given element.
 **/
static inline void
list_insert(list_node_t *prev, list_node_t *node)
{
	node->next = prev->next;
	node->prev = prev;
	node->prev->next = node;
	node->next->prev = node;
}

/**
 * Remove an element from a list.
 **/
static inline void
list_remove(list_node_t *node)
{
	node->next->prev = node->prev;
	node->prev->next = node->next;
	list_init(node);
}

#ifdef __cplusplus
}
#endif

#endif /* UTIL_H */

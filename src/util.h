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

#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <errno.h>

#include <GL/gl.h>

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

#define foreach(head_, pos_) \
	for (list_node_t *head = (head_), *pos_ = head->next; \
	     pos_ != head; pos_ = pos_->next)

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
 * Remove an element from a list.
 **/
static inline void
list_remove(list_node_t *node)
{
	node->next->prev = node->prev;
	node->prev->next = node->next;
	list_init(node);
}

/**
 * Insert an element into a list after the given element.
 **/
static inline void
list_insert(list_node_t *prev, list_node_t *node)
{
	list_remove(node);
	node->next = prev->next;
	node->prev = prev;
	node->prev->next = node;
	node->next->prev = node;
}

/**
 * Just like read(2) but with no risk of EINTR and aborting on other errors.
 **/
static inline size_t
xread(int fd, void *buf, size_t count)
{
	ssize_t got;
	size_t total = 0;

	while (count) {
		got = read(fd, buf, count);

		if (! got)
			break;

		if (got < 0 && errno != EINTR)
			errx(1, "error during read");

		if (got < 0)
			got = 0;

		total += got;
		count -= got;
		buf = (char *)buf + got;
	}

	return total;
}

/**
 * Abort-on-error version of dup.
 **/
static inline int
xdup(int fd)
{
	int ret = -1;
	errno = EINTR;

	while (errno == EINTR && ret == -1)
		ret = dup(fd);

	if (ret < 0)
		errx(1, "Could not duplicate file descriptor");

	return ret;
}

/**
 * When allocating a potentially expanding array of elements, how many elements
 * should we have space for minimum.
 **/
#define VEC_BASE_SIZE 8

/**
 * Expand an allocated array to fit at least one more element.
 *
 * vec: Array to operate on.
 * items: Number of used items in the array.
 * item_sz: Size of each item.
 **/
static inline void *
do_vec_expand(void *vec, size_t items, size_t item_sz)
{
	if (! vec)
		return xcalloc(VEC_BASE_SIZE, item_sz);

	if (items < VEC_BASE_SIZE)
		return vec;

	if (items & (items - 1))
		return vec;

	return xrealloc(vec, items * 2 * item_sz);
}

/**
 * Contract an allocated array to be at least 66% used.
 *
 * vec: Array to operate on.
 * items: Number of used items in the array.
 * item_sz: Size of each item.
 **/
static inline void *
do_vec_contract(void *vec, size_t items, size_t item_sz)
{
	size_t expected = items + items / 2;
	size_t i;

	for (i = sizeof(size_t) * 4; i; i /= 2)
		expected |= expected >> i;

	expected++;

	if (expected < VEC_BASE_SIZE)
		expected = VEC_BASE_SIZE;

	return xrealloc(vec, expected * item_sz);
}

#define vec_expand(x, y) do_vec_expand((x), (y), sizeof(*(x)))
#define vec_contract(x, y) do_vec_contract((x), (y), sizeof(*(x)))

/**
 * Check whether an OpenGL error has occurred and die if it has.
 *
 * mem_ok: If set, return -1 for GL_OUT_OF_MEMORY instead of dying.
 * file, line: File name and line number for error reporting.
 *
 * Returns: 0 unless mem_ok is set.
 **/
static inline int
check_gl_ok(int mem_ok, const char *file, size_t line)
{
	GLenum error = glGetError();

	if (error == GL_NO_ERROR)
		return 0;

	if (error == GL_OUT_OF_MEMORY && mem_ok)
		return -1;

	if (error == GL_OUT_OF_MEMORY)
		errx(1, "OpenGL ran out of memory at %s: %zd", file, line);

	if (error == GL_INVALID_ENUM)
		errx(1, "OpenGL encountered an invalid GLenum at %s: %zd",
		     file, line);

	if (error == GL_INVALID_VALUE)
		errx(1, "OpenGL encountered an out of range value at %s:%zd",
		     file, line);

	if (error == GL_INVALID_OPERATION)
		errx(1, "OpenGL's state did not allow the "
		     "requested operation at %s:%zd", file, line);

	if (error == GL_INVALID_FRAMEBUFFER_OPERATION)
		errx(1, "OpenGL encountered an incomplete "
		     "framebuffer at %s:%zd", file, line);

	if (error == GL_STACK_UNDERFLOW)
		errx(1, "OpenGL encountered an internal stack "
		     "underflow at %s:%zd", file, line);

	if (error == GL_STACK_OVERFLOW)
		errx(1, "OpenGL encountered an internal stack "
		     "underflow at %s:%zd", file, line);

	errx(1, "OpenGL encountered an unrecognized error code %d at %s:%zd",
	     error, file, line);
}

#define CHECK_GL_MEM check_gl_ok(1, __FILE__, __LINE__)
#define CHECK_GL check_gl_ok(0, __FILE__, __LINE__)
#define CLEAR_GL glGetError()

#ifdef __cplusplus
}
#endif

#endif /* UTIL_H */

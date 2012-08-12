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
#include <err.h>

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

#endif /* UTIL_H */

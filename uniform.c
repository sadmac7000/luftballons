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

#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "uniform.h"
#include "vbuf.h"
#include "util.h"

/**
 * Destructor for a shader uniform.
 **/
static void
uniform_destructor(void *v)
{
	uniform_t *uniform = v;

	free(uniform->name);
	free(uniform);
}

/**
 * Create a uniform object.
 **/
uniform_t *
uniform_create(const char *name, uniform_type_t type, uniform_value_t value)
{
	uniform_t *ret = xmalloc(sizeof(uniform_t));

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, uniform_destructor, ret);
	ret->type = type;
	ret->value = value;
	ret->name = xstrdup(name);

	return ret;
}

/**
 * Grab a uniform.
 **/
void
uniform_grab(uniform_t *uniform)
{
	refcount_grab(&uniform->refcount);
}

/**
 * Ungrab a uniform.
 **/
void
uniform_ungrab(uniform_t *uniform)
{
	refcount_ungrab(&uniform->refcount);
}

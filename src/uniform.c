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

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "uniform.h"
#include "util.h"
#include "texmap.h"

/**
 * Destructor for a shader uniform.
 **/
static void
uniform_destructor(void *v)
{
	uniform_t *uniform = v;

	if (uniform->type == UNIFORM_SAMP2D)
		texmap_ungrab(uniform->value.data_ptr);

	free(uniform->name);
	free(uniform);
}

/**
 * Create a uniform object.
 **/
uniform_t *
uniform_create(const char *name, uniform_type_t type, ...)
{
	uniform_t *ret = xmalloc(sizeof(uniform_t));
	uniform_value_t value;
	va_list ap;

	va_start(ap, type);
	
	switch (type) {
	case UNIFORM_MAT4:
	case UNIFORM_VEC4:
	case UNIFORM_SAMP2D:
	case UNIFORM_SAMP1D:
		value.data_ptr = va_arg(ap, void *);
		break;
	case UNIFORM_UINT:
		value.uint = va_arg(ap, GLuint);
		break;
	default:
		errx(1, "Must specify a valid uniform type "
		     "when creating a uniform");
	}

	va_end(ap);

	refcount_init(&ret->refcount);
	refcount_add_destructor(&ret->refcount, uniform_destructor, ret);
	ret->type = type;
	ret->value = value;
	ret->name = xstrdup(name);

	if (type == UNIFORM_SAMP2D)
		texmap_grab(ret->value.data_ptr);

	return ret;
}
EXPORT(uniform_create);

/**
 * Grab a uniform.
 **/
void
uniform_grab(uniform_t *uniform)
{
	refcount_grab(&uniform->refcount);
}
EXPORT(uniform_grab);

/**
 * Ungrab a uniform.
 **/
void
uniform_ungrab(uniform_t *uniform)
{
	refcount_ungrab(&uniform->refcount);
}
EXPORT(uniform_ungrab);

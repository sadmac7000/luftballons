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

	if (uniform->type == UNIFORM_TEXMAP)
		texmap_ungrab(uniform->value.data_ptr);

	free(uniform->name);
	free(uniform);
}

/**
 * Create a uniform object.
 *
 * type: The type of uniform to create.
 *
 * If type is LUFT_UNIFORM_CLONE:
 *   uniform: A uniform_t to clone
 * Otherwise:
 *   name: The name of the uniform to create.
 *   value: The value to assign to the uniform.
 **/
uniform_t *
uniform_create(uniform_type_t type, ...)
{
	uniform_t *ret;
	va_list ap;

	va_start(ap, type);
	
	ret = uniform_vcreate(type, ap);

	va_end(ap);


	return ret;
}
EXPORT(uniform_create);

/**
 * Create a uniform object. Use a va_list for the final indeterminite argument.
 **/
uniform_t *
uniform_vcreate(uniform_type_t type, va_list ap)
{
	uniform_t *ret;
	uniform_value_t value;
	const char *name;

	if (type == UNIFORM_CLONE) {
		ret = va_arg(ap, uniform_t *);
		uniform_grab(ret);
		return ret;
	}

	name = va_arg(ap, const char *);

	switch (type) {
	case UNIFORM_MAT4:
		value.data_ptr = xmemdup(va_arg(ap, void *),
					 16 * sizeof(float));
		break;
	case UNIFORM_VEC4:
		value.data_ptr = xmemdup(va_arg(ap, void *),
					 4 * sizeof(float));
		break;
	case UNIFORM_TEXMAP:
		value.data_ptr = va_arg(ap, void *);
		texmap_grab(value.data_ptr);
		break;
	case UNIFORM_UINT:
		value.uint = va_arg(ap, GLuint);
		break;
	default:
		errx(1, "Must specify a valid uniform type "
		     "when creating a uniform");
	}

	ret = xmalloc(sizeof(uniform_t));

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

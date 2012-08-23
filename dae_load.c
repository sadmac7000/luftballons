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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <expat.h>
#include <err.h>
#include <string.h>

#include "dae_load.h"
#include "util.h"
#include "mesh.h"

struct dae_parse_state;
typedef void (*dae_start_t)(struct dae_parse_state *state, const char *el,
			    const char **attrs);
typedef void (*dae_end_t)(struct dae_parse_state *state, const char *el);
typedef void (*dae_chardata_t)(struct dae_parse_state *state,
			       const XML_Char *s, int len);

/**
 * Stack frame for the parse stack.
 **/
typedef struct dae_parse_frame {
	dae_start_t start;
	dae_end_t end;
	dae_chardata_t text;
	struct dae_parse_frame *prev;
	size_t count;
} dae_parse_frame_t;

/**
 * State of a parsing operation.
 *
 * frame: Top stack frame.
 * file: Name of file we're parsing.
 * count: Number of elements deep we are.
 * skip_count: Count at which we resume processing elements.
 **/
typedef struct dae_parse_state {
	dae_parse_frame_t *frame;
	const char *file;
	size_t count;
	size_t skip_count;
} dae_parse_state_t;

/**
 * Tell parser to skip tags under the current tag.
 **/
static void
dae_parse_skip(dae_parse_state_t *state)
{
	if (state->skip_count)
		warnx("Nested calls to dae_parse_skip");

	state->skip_count = state->count;
}

/**
 * Push a new frame onto the parse state.
 **/
static void
dae_parse_state_push(dae_parse_state_t *state, dae_start_t start,
		     dae_end_t end, dae_chardata_t text)
{
	dae_parse_frame_t *new = xmalloc(sizeof(dae_parse_frame_t));

	new->prev = state->frame;
	state->frame = new;

	new->start = start;
	new->end = end;
	new->text = text;
	new->count = state->count;
}

/**
 * Pop a frame from the parse state.
 **/
static void
dae_parse_state_pop(dae_parse_state_t *state)
{
	dae_parse_frame_t *old = state->frame;
	
	state->frame = old->prev;

	free(old);
}

/**
 * Callback for expat for the start of an element.
 **/
void
dae_elem_start(void *data, const char *el, const char **attr)
{
	dae_parse_state_t *state = data;

	state->count++;

	if (state->skip_count > 0 && state->count > state->skip_count)
		return;

	if (state->frame->start)
		state->frame->start(data, el, attr);
}

/**
 * Callback for expat for the end of an element.
 **/
void
dae_elem_end(void *data, const char *el)
{
	dae_parse_state_t *state = data;

	if (state->count == state->frame->count)
		dae_parse_state_pop(state);
	else if (!state->skip_count && state->frame->end)
		state->frame->end(data, el);

	if (state->skip_count == state->count)
		state->skip_count = 0;

	state->count--;
}

/**
 * Callback for expat for character data.
 **/
void
dae_elem_chardata(void *data, const XML_Char *s, int len)
{
	dae_parse_state_t *state = data;

	if (strspn(s, " \t\n\r\v") == (size_t)len)
		return;

	if (state->skip_count)
		return;

	if (state->frame->text)
		state->frame->text(data, s, len);
	else if (! state->skip_count)
		warnx("%s contains unexpected data", state->file);
}

/**
 * Geometry element start handler.
 **/
static void
dae_start_geom(dae_parse_state_t *state, const char *el, const char **attrs)
{
	(void)state;
	(void)attrs;
	printf("%s\n", el);
}

/**
 * Geometry element end handler.
 **/
static void
dae_end_geom(dae_parse_state_t *state, const char *el)
{
	printf("/%s\n", el);
}

/**
 * Initial start handler.
 **/
static void
dae_start_collada(dae_parse_state_t *state, const char *el, const char **attrs)
{
	(void)attrs;

	if (! strcmp(el, "library_geometries"))
		dae_parse_state_push(state, dae_start_geom, dae_end_geom, NULL);
	else if (strcmp(el, "COLLADA"))
		dae_parse_skip(state);
}

/**
 * Initial end handler.
 **/
static void
dae_end_collada(dae_parse_state_t *state, const char *el)
{
	return;
}

/**
 * Load an object or series of objects from a string of COLLADA data.
 *
 * filename: Name of the file the data comes from.
 * data: Data to load.
 * len: Length of `data`.
 * count: Place to store number of objects gotten.
 * 
 * Returns: An array of gotten objects.
 **/
static object_t **
dae_do_load(const char *filename, const char *data, size_t len, size_t *count)
{
	dae_parse_state_t state;
	XML_Parser parser = XML_ParserCreate(NULL);

	*count = 0;
	state.file = filename;
	state.frame = NULL;
	state.count = 0;
	state.skip_count = 0;

	XML_SetUserData(parser, &state);
	XML_SetElementHandler(parser, dae_elem_start, dae_elem_end);
	XML_SetCharacterDataHandler(parser, dae_elem_chardata);

	dae_parse_state_push(&state, dae_start_collada, dae_end_collada, NULL);

	if (! XML_Parse(parser, data, len, 1))
		errx(1, "Could not parse COLLADA data from %s (line %lu): %s",
		     filename, XML_GetCurrentLineNumber(parser),
		     XML_ErrorString(XML_GetErrorCode(parser)));

	return NULL;
}

/**
 * Load an object or series of objects from a COLLADA file.
 *
 * filename: Name of file to load.
 * count: Place to store number of objects gotten.
 * 
 * Returns: An array of gotten objects.
 **/
object_t **
dae_load(const char *filename, size_t *count)
{
	int fd;
	const char *buf;
	ssize_t filesz;
	object_t **ret;

	fd = open(filename, O_CLOEXEC | O_RDONLY);

	if (fd < 0)
		err(1, "Could not open model file '%s'", filename);

	filesz = lseek(fd, 0, SEEK_END);

	if (filesz < 0)
		err(1, "Could not get length of file '%s'", filename);

	buf = mmap(NULL, filesz, PROT_READ, MAP_PRIVATE, fd, 0);

	if (! buf)
		err(1, "Could not map file '%s'", filename);

	ret = dae_do_load(filename, buf, filesz, count);

	munmap((void *)buf, filesz);
	return ret;
}

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
#include <err.h>
#include <string.h>
#include <ctype.h>

#include <dae.h>
#include <dom/domCOLLADA.h>
#include <dom/domMesh.h>
#include <dom/domGeometry.h>
#include <dom/domInputLocal.h>

#include "dae_load.h"
#include "util.h"
#include "mesh.h"
#include "vbuf.h"

using namespace std;

/**
 * Get the number of parameters an accessor takes and uses.
 **/
static size_t
dae_get_accessor_size(domAccessorRef accessor)
{
	domParam_Array params = accessor->getParam_array();
	const char *name;
	size_t i;
	size_t ret = 0;

	for (i = 0; i < params.getCount(); i++) {
		name = params[i]->getName();

		if (name && strlen(name))
			ret++;
	}

	return ret;
}

/**
 * Given a vertex input, find the position input.
 **/
static domInputLocalRef
dae_vert_to_pos(domInputLocalOffsetRef input)
{
	domVerticesRef ref = input->getSource().getElement();
	domInputLocal_Array array = ref->getInput_array();
	size_t i;
	const char *c;

	for (i = 0; i < array.getCount(); i++) {
		c = array[i]->getSemantic();
		if (strcmp(c, "POSITION"))
			warnx("Discarding COLLADA vertex input %s", c);
		else
			return array[i];
	}

	errx(1, "COLLADA document had vertices with no position");
}

/**
 * Load a single vertex data source.
 **/
static domAccessorRef
dae_load_source(domInputLocalOffsetRef input, vbuf_fmt_t *seg)
{
	domSource::domTechnique_commonRef tech;
	domInputLocalRef vinput;
	domFloat_arrayRef farray;
	domInt_arrayRef iarray;
	domAccessorRef accessor;
	daeElementRef source_data;
	xsInteger min, max;
	char *c;
	const char *semantic = input->getSemantic();
	domSourceRef source = input->getSource().getElement();

	if (! strcmp(semantic, "VERTEX")) {
		vinput = dae_vert_to_pos(input);
		semantic = vinput->getSemantic();
		source = vinput->getSource().getElement();
	}

	strncpy(seg->name, semantic, 31);
	seg->name[31] = '\0';
	
	for (c = seg->name; *c; *c = tolower(*c), c++);

	if (source->typeID() != domSource::ID())
		errx(1, "COLLADA vertex input points to source that's not a"
		     " source");

	tech = source->getTechnique_common();

	if (! tech)
		errx(1, "COLLADA source without technique_common"
		     " unimplemented");

	accessor = tech->getAccessor();
	seg->size = dae_get_accessor_size(accessor);

	source_data = accessor->getSource().getElement();

	if (source_data->typeID() == domBool_array::ID()) {
		seg->type = GL_UNSIGNED_BYTE;
	} else if (source_data->typeID() == domInt_array::ID()) {
		iarray = (domInt_arrayRef)source_data;
		min = iarray->getMinInclusive();
		max = iarray->getMaxInclusive();

		if (min >= 0 && max < 256)
			seg->type = GL_UNSIGNED_BYTE;
		else if (min >= 0 && max < 65536)
			seg->type = GL_UNSIGNED_SHORT;
		else if (min >= 0)
			seg->type = GL_UNSIGNED_INT;
		else if (max < 128)
			seg->type = GL_BYTE;
		else if (max < 32768)
			seg->type = GL_SHORT;
		else
			seg->type = GL_INT;
	} else if (source_data->typeID() == domFloat_array::ID()) {
		seg->type = GL_FLOAT;
		farray = (domFloat_arrayRef)source_data;

		/* FIXME: We're using doubles for anything more precise than
		 * default right now. Should probably calculate implied
		 * bit precision from the two numbers.
		 */
		if (farray->getDigits() > 6 || farray->getMagnitude() > 38)
			seg->type = GL_DOUBLE;
	} else {
		errx(1, "Unsupported COLLADA source type");
	}

	return accessor;
}

/**
 * Copy vertex data into the buffer.
 **/
static void * 
dae_copy_data(domAccessorRef accessor, GLenum type, size_t idx, void *buffer)
{
	daeElementRef source;
	domParam_Array params = accessor->getParam_array();
	size_t i;
	size_t stride = accessor->getStride();
	const char *name;

	source = accessor->getSource().getElement();

#define ASSIGNMENT_LOOP(type) ({				\
	type *target = (type *)buffer;				\
								\
	for (i = 0; i < params.getCount(); i++) {		\
		name = params[i]->getName();			\
		if (name && strlen(name))			\
			*(target++) = array[idx * stride + i];	\
	}							\
								\
	return (void *)target;					\
})

	if (source->typeID() == domBool_array::ID()) {
		domListOfBools array = ((domBool_arrayRef)source)->getValue();
		ASSIGNMENT_LOOP(unsigned char);
	} else if (source->typeID() == domFloat_array::ID()) {
		domListOfFloats array = ((domFloat_arrayRef)source)->getValue();

		if (type == GL_DOUBLE) {
			ASSIGNMENT_LOOP(double);
		} else {
			ASSIGNMENT_LOOP(float);
		}
	} else if (source->typeID() == domInt_array::ID()) {
		domListOfInts array = ((domInt_arrayRef)source)->getValue();

		if (type == GL_UNSIGNED_BYTE)
			ASSIGNMENT_LOOP(unsigned char);
		else if (type == GL_UNSIGNED_SHORT)
			ASSIGNMENT_LOOP(unsigned short);
		else if (type == GL_UNSIGNED_INT)
			ASSIGNMENT_LOOP(unsigned int);
		else if (type == GL_BYTE)
			ASSIGNMENT_LOOP(signed char);
		else if (type == GL_SHORT)
			ASSIGNMENT_LOOP(short);
		else if (type == GL_INT)
			ASSIGNMENT_LOOP(int);
	}

	errx(1, "Got strange source data while copying vertex data");
}

/**
 * Load a mesh containing a polylist.
 *
 * mesh: The mesh to load.
 *
 * Returns: A new object_t pointer or NULL if this mesh doesn't contain a
 * polylist.
 **/
static object_t *
dae_load_polylist(domMeshRef mesh)
{
	domPolylistRef polylist;
	domPolylist_Array pa;
	domVerticesRef verts;
	domInputLocalOffset_Array inputs;
	domListOfUInts vcounts;
	domListOfUInts indices;
	size_t i;
	size_t vert_count;
	size_t bufsize;
	vbuf_fmt_t *fmt;
	size_t input_count;
	size_t input_stride;
	domSourceRef source;
	daeTArray<domAccessorRef> sources;
	void *data;
	size_t j;

	pa = mesh->getPolylist_array();
	if (! pa.getCount())
		return NULL;

	polylist = pa[0];

	inputs = polylist->getInput_array();
	input_count = inputs.getCount();

	fmt = (vbuf_fmt_t *)xcalloc(input_count, sizeof (vbuf_fmt_t));

	input_stride = 0;
	for (i = 0; i < input_count; i++) {
		sources.append(dae_load_source(inputs[i], &fmt[i]));
		
		if (inputs[i]->getOffset() > input_stride)
			input_stride = inputs[i]->getOffset();
	}
	input_stride++;

	vcounts = polylist->getVcount()->getValue();
	vert_count = 0;

	for (i = 0; i < vcounts.getCount(); i++) {
		if (vcounts[i] == 3) {
			vert_count += vcounts[i];
			continue;
		}

		warnx("COLLADA Polylist contains non-triangle"
		      " with %llu sides", vcounts[i]);

		free(fmt);
		return NULL;
	}

	bufsize = 0;

	for (i = 0; i < input_count; i++)
		bufsize += vbuf_segment_size(&fmt[i]) * vert_count;

	data = xcalloc(1, bufsize);
	bufsize = 0;

	indices = polylist->getP()->getValue();
	for (i = 0; i < input_count; i++)
		for (j = inputs[i]->getOffset(); j < vert_count;
		     j += input_stride)
			data = dae_copy_data(sources[i], fmt[i].type,
					     indices[j], data);

	return NULL;
}

/**
 * Turn a COLLADA geometry node into an object_t.
 * 
 * geo: The geometry node to load.
 *
 * Returns: A new object_t pointer or NULL if this geometry isn't a mesh.
 **/
static object_t *
dae_load_geom(domGeometry &geo)
{
	domMeshRef mesh = geo.getMesh();
	object_t *ret;

	if (! mesh)
		return NULL;

	ret = dae_load_polylist(mesh);

	if (ret)
		return ret;

	/* Loaders for tri-fans etc go here */

	return NULL;
}

extern "C" {

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
	DAE dae;
	daeElement *root;
	domCOLLADA *doc;
	domLibrary_geometries_Array libs;
	domGeometry_Array geoms;
	object_t **ret;
	size_t n_geoms;
	size_t i;

	root = dae.open(filename);
	if (! root)
		errx(1, "Could not load COLLADA file %s", filename);

	if (root->typeID() != domCOLLADA::ID())
		errx(1, "COLLADA type mismatch: %d != %d in %s", root->typeID(),
		     domCOLLADA::ID(), filename);

	doc = (domCOLLADA *)root;

	libs = doc->getLibrary_geometries_array();
	if (libs.getCount() != 1)
		errx(1, "Bad geometry library count %lu in COLLADA file %s",
		     libs.getCount(), filename);

	geoms = libs[0]->getGeometry_array();
	n_geoms = geoms.getCount();
	ret = (object_t **)xcalloc(n_geoms, sizeof(object_t *));
	*count = 0;

	for (i = 0; i < n_geoms; i++) {
		ret[*count] = dae_load_geom(*geoms[i]);

		if (ret[*count])
			(*count)++;
	}

	if (*count == n_geoms)
		return ret;

	if (*count)
		return (object_t **)xrealloc(ret, *count * sizeof(object_t *));

	free(ret);
	return NULL;
}

} /* extern "C" */

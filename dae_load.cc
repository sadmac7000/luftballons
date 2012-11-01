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
#include <time.h>

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
dae_load_source(domInputLocalOffsetRef input, vbuf_fmt_t *fmt)
{
	domSource::domTechnique_commonRef tech;
	domInputLocalRef vinput;
	domFloat_arrayRef farray;
	domInt_arrayRef iarray;
	domAccessorRef accessor;
	daeElementRef source_data;
	xsInteger min, max;
	char *c;
	char *name = xstrdup(input->getSemantic());
	size_t elems;
	domSourceRef source = input->getSource().getElement();

	if (! strcmp(name, "VERTEX")) {
		free(name);
		vinput = dae_vert_to_pos(input);
		name = xstrdup(vinput->getSemantic());
		source = vinput->getSource().getElement();
	}

	for (c = name; *c; *c = tolower(*c), c++);

	if (source->typeID() != domSource::ID())
		errx(1, "COLLADA vertex input points to source that's not a"
		     " source");

	tech = source->getTechnique_common();

	if (! tech)
		errx(1, "COLLADA source without technique_common"
		     " unimplemented");

	accessor = tech->getAccessor();
	elems = dae_get_accessor_size(accessor);

	source_data = accessor->getSource().getElement();

	if (source_data->typeID() == domBool_array::ID()) {
		vbuf_fmt_add(fmt, name, elems, GL_UNSIGNED_BYTE);
	} else if (source_data->typeID() == domInt_array::ID()) {
		iarray = (domInt_arrayRef)source_data;
		min = iarray->getMinInclusive();
		max = iarray->getMaxInclusive();

		if (min >= 0 && max < 256)
			vbuf_fmt_add(fmt, name, elems, GL_UNSIGNED_BYTE);
		else if (min >= 0 && max < 65536)
			vbuf_fmt_add(fmt, name, elems, GL_UNSIGNED_SHORT);
		else if (min >= 0)
			vbuf_fmt_add(fmt, name, elems, GL_UNSIGNED_INT);
		else if (max < 128)
			vbuf_fmt_add(fmt, name, elems, GL_BYTE);
		else if (max < 32768)
			vbuf_fmt_add(fmt, name, elems, GL_SHORT);
		else
			vbuf_fmt_add(fmt, name, elems, GL_INT);
	} else if (source_data->typeID() == domFloat_array::ID()) {
		farray = (domFloat_arrayRef)source_data;

		/* FIXME: We're using doubles for anything more precise than
		 * default right now. Should probably calculate implied
		 * bit precision from the two numbers.
		 */
		if (farray->getDigits() > 6 || farray->getMagnitude() > 38)
			vbuf_fmt_add(fmt, name, elems, GL_DOUBLE);
		else
			vbuf_fmt_add(fmt, name, elems, GL_FLOAT);
	} else {
		errx(1, "Unsupported COLLADA source type");
	}

	return accessor;
}

/**
 * Copy vertex data into the buffer.
 **/
static void * 
dae_copy_data(domParam_Array params, size_t stride, daeElementRef source,
	      GLenum type, size_t idx, void *buffer)
{
	size_t i;
	const char *name;

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
	vbuf_fmt_t fmt = 0;
	vbuf_fmt_t iter;
	size_t input_count;
	size_t input_stride;
	domSourceRef source;
	daeTArray<domAccessorRef> sources;
	void *data;
	uint16_t *ebuf;
	void *loc;
	size_t j;
	mesh_t *out_mesh;
	GLenum type;

	pa = mesh->getPolylist_array();
	if (! pa.getCount())
		return NULL;

	polylist = pa[0];

	inputs = polylist->getInput_array();
	input_count = inputs.getCount();

	input_stride = 0;
	for (i = 0; i < input_count; i++) {
		sources.append(dae_load_source(inputs[i], &fmt));
		
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

		return NULL;
	}

	if (vert_count >= 1 << 16)
		errx(1, "Cannot load COLLADA mesh with %lu vertices (max %hu)",
		     vert_count, ~(uint16_t)0);

	bufsize = vbuf_fmt_vert_size(fmt) * vert_count;

	data = xcalloc(1, bufsize);
	loc = data;
	bufsize = 0;

	indices = polylist->getP()->getValue();
	iter = fmt;
	i = 0;
	while (vbuf_fmt_pop_segment(&iter, NULL, &type, NULL, NULL)) {
		domParam_Array params = sources[i]->getParam_array();
		size_t stride = sources[i]->getStride();
		daeElementRef source = sources[i]->getSource().getElement();

		for (j = inputs[i]->getOffset(); j < vert_count * input_stride;
		     j += input_stride) {
			loc = dae_copy_data(params, stride, source, type,
					    indices[j], loc);
		}

		i++;
	}

	ebuf = (uint16_t *)xcalloc(vert_count, sizeof(uint16_t));

	for (i = 0; i < vert_count; i++)
		ebuf[i] = i;

	out_mesh = mesh_create(vert_count, (float *)data, vert_count,
			       (uint16_t *)ebuf, fmt, GL_TRIANGLES);

	free(data);
	free(ebuf);

	return object_create(out_mesh, NULL);
}

/**
 * Turn a COLLADA geometry node into an object_t.
 * 
 * geo: The geometry node to load.
 *
 * Returns: A new object_t pointer or NULL if this geometry isn't a mesh.
 **/
static object_t *
dae_load_geom(domGeometryRef geo)
{
	domMeshRef mesh = geo->getMesh();
	object_t *ret;

	if (! mesh)
		return NULL;

	ret = dae_load_polylist(mesh);

	if (ret)
		return ret;

	/* Loaders for tri-fans etc go here */

	return NULL;
}

/**
 * Given a domNode and an object, apply all transforms specified in that node
 * to the object.
 **/
static void
dae_apply_transform(domNodeRef node, object_t *object)
{
	(void)node;
	(void)object;
}

/**
 * Given a domNode eleement to specify it, create a new object with the given
 * parent.
 **/
static object_t *
dae_process_node(domNodeRef node, object_t *parent)
{
	domInstance_geometry_Array arr = node->getInstance_geometry_array();
	object_t *object;
	domNode_Array children;
	size_t i;

	if (arr.getCount() > 1)
		errx(1, "COLLADA import supports exactly 1 mesh per node."
		     " Found %lu", arr.getCount());

	if (arr.getCount() == 0) {
		object = object_create(NULL, parent);
		goto out;
	}

	object = dae_load_geom(arr[0]->getUrl().getElement());

	if (! object)
		return NULL;

	if (parent)
		object_add_child(parent, object);

	dae_apply_transform(node, object);

out:
	children = node->getNode_array();

	for (i = 0; i < children.getCount(); i++)
		dae_process_node(children[i], object);

	return object;
}

/**
 * Process COLLADA nodes from a visual scene.
 **/
static void
dae_process_scenes(domVisual_scene_Array &scenes,
		   daeTArray<object_t *> &output)
{
	domNode_Array nodes;
	size_t i;
	size_t j;
	object_t *obj;

	for (i = 0; i < scenes.getCount(); i++) {
		nodes = scenes[i]->getNode_array();

		for (j = 0; j < nodes.getCount(); j++) {
			obj = dae_process_node(nodes[j], NULL);
			if (obj)
				output.append(obj);
		}
	}
}

/**
 * Process COLLADA nodes from the visual scene library
 **/
static void
dae_get_nodes_scenes(domCOLLADA *doc, daeTArray<object_t *> &output)
{
	domLibrary_visual_scenes_Array lib;
	domVisual_scene_Array scenes;
	size_t i;
	size_t j;

	lib = doc->getLibrary_visual_scenes_array();

	for (i = 0; i < lib.getCount(); i++) {
		scenes = lib[i]->getVisual_scene_array();

		for (j = 0; j < scenes.getCount(); j++)
			dae_process_scenes(scenes, output);
	}
}

/**
 * Process COLLADA nodes from the node library
 **/
static void
dae_get_nodes_lib(domCOLLADA *doc, daeTArray<object_t *> &output)
{
	domLibrary_nodes_Array lib;
	domNode_Array nodes;
	size_t i;
	size_t j;
	object_t *obj;

	lib = doc->getLibrary_nodes_array();

	for (i = 0; i < lib.getCount(); i++) {
		nodes = lib[i]->getNode_array();

		for (j = 0; j < nodes.getCount(); j++) {
			obj = dae_process_node(nodes[j], NULL);
			if (obj)
				output.append(obj);
		}
	}
}

/**
 * Get COLLADA nodes elements and process each of them
 **/
static object_t **
dae_get_nodes(domCOLLADA *doc, size_t *count)
{
	daeTArray<object_t *> output;
	object_t **ret;
	size_t i;

	dae_get_nodes_scenes(doc, output);
	dae_get_nodes_lib(doc, output);

	*count = output.getCount();
	if (! output.getCount())
		return NULL;

	ret = (object_t **)xcalloc(*count, sizeof(object_t *));

	for (i = 0; i < *count; i++)
		ret[i] = output[i];

	return ret;
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

	root = dae.open(filename);
	if (! root)
		errx(1, "Could not load COLLADA file %s", filename);

	if (root->typeID() != domCOLLADA::ID())
		errx(1, "COLLADA type mismatch: %d != %d in %s", root->typeID(),
		     domCOLLADA::ID(), filename);

	doc = (domCOLLADA *)root;

	return dae_get_nodes(doc, count);
}

} /* extern "C" */

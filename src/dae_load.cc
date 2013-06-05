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

#include "object.h"
#include "util.h"
#include "mesh.h"
#include "matrix.h"
#include "vbuf.h"
#include "quat.h"

using namespace std;

/**
 * A data source. Pulls streams of data out of COLLADA files.
 **/
class DAEDataSource {

/* datum_sz: Size of a single datum. I.e. one axis in a stream of coordinates.
 * name: Name of the type of data this stream provides.
 * gl_type: OpenGL type of each datum.
 * data: A buffer that will be filled with the data for this stream.
 * params_enabled: List of what parameters are enabled.
 * param_count: Number of parameters (3 for an xyz coordinate, etc.)
 * used_param_count: Number of parameters we aren't supposed to ignore.
 * stride: How many datum in a data item.
 * source: the COLLADA source element we are using.
 **/
size_t datum_sz;
char *name;
GLenum gl_type;
void *data;
daeTArray<int> params_enabled;
size_t param_count;
size_t used_param_count;
size_t stride;
daeElementRef source;

/**
 * Given a vertex input, find the position input.
 **/
domInputLocalRef
vert_to_pos(domInputLocalOffsetRef input)
{
	domVerticesRef ref = input->getSource().getElement();
	domInputLocal_Array array = ref->getInput_array();
	size_t i;
	const char *c;

	for (i = 0; i < array.getCount(); i++) {
		c = array[i]->getSemantic();
		if (! strcmp(c, "POSITION"))
			return array[i];
		warnx("Discarding COLLADA vertex input %s", c);
	}

	errx(1, "COLLADA document had vertices with no position");
}

/**
 * Set the value of gl_type and load data. Assume we have an integer type source.
 **/
void
set_gl_type_int()
{
	domInt_arrayRef array = (domInt_arrayRef)this->source;
	int min = array->getMinInclusive();
	int max = array->getMaxInclusive();

	if (min >= 0 && max < 256) {
		this->gl_type = GL_UNSIGNED_BYTE;
		this->grab_buffer<domListOfInts, unsigned char>(((domInt_arrayRef)source)->getValue());
		this->datum_sz = sizeof(unsigned char);
	} else if (min >= 0 && max < 65536) {
		this->gl_type = GL_UNSIGNED_SHORT;
		this->grab_buffer<domListOfInts, unsigned short>(((domInt_arrayRef)source)->getValue());
		this->datum_sz = sizeof(unsigned short);
	} else if (min >= 0) {
		this->gl_type = GL_UNSIGNED_INT;
		this->grab_buffer<domListOfInts, unsigned int>(((domInt_arrayRef)source)->getValue());
		this->datum_sz = sizeof(unsigned int);
	} else if (max < 128) {
		this->gl_type = GL_BYTE;
		this->grab_buffer<domListOfInts, signed char>(((domInt_arrayRef)source)->getValue());
		this->datum_sz = sizeof(signed char);
	} else if (max < 32768) {
		this->gl_type = GL_SHORT;
		this->grab_buffer<domListOfInts, short>(((domInt_arrayRef)source)->getValue());
		this->datum_sz = sizeof(short);
	} else {
		this->gl_type = GL_INT;
		this->grab_buffer<domListOfInts, int>(((domInt_arrayRef)source)->getValue());
		this->datum_sz = sizeof(int);
	}
}

/**
 * Set the value of gl_type and load data. Assume we have a float type source.
 **/
void
set_gl_type_float()
{
	domFloat_arrayRef array = (domFloat_arrayRef)this->source;

	/* FIXME: We're using doubles for anything more precise than
	 * default right now. Should probably calculate implied
	 * bit precision from the two numbers.
	 */
	if (array->getDigits() > 6 || array->getMagnitude() > 38) {
		this->gl_type = GL_DOUBLE;
		this->grab_buffer<domListOfFloats, double>(((domFloat_arrayRef)source)->getValue());
		this->datum_sz = sizeof(double);
	} else {
		this->gl_type = GL_FLOAT;
		this->grab_buffer<domListOfFloats, float>(((domFloat_arrayRef)source)->getValue());
		this->datum_sz = sizeof(float);
	}
}

/**
 * Set the value of gl_type and load data.
 **/
void
set_gl_type()
{
	if (this->source->typeID() == domBool_array::ID()) {
		this->gl_type = GL_UNSIGNED_BYTE;
		this->grab_buffer<domListOfBools, unsigned char>(((domBool_arrayRef)source)->getValue());
		this->datum_sz = sizeof(unsigned char);
	} else if (this->source->typeID() == domInt_array::ID()) {
		this->set_gl_type_int();
	} else if (this->source->typeID() == domFloat_array::ID()) {
		this->set_gl_type_float();
	} else {
		errx(1, "Unsupported COLLADA source type");
	}
}

/**
 * Fill out our data buffer with accessor contents from a COLLADA file.
 **/
template <typename T, typename U>
void
grab_buffer(T array)
{
	size_t array_size = array.getCount();
	U *buffer = (U *)xcalloc(array_size, sizeof(U));
	size_t i;
	size_t mod;
	size_t j = 0;

	for (i = 0; i < array_size; i++) {
		mod = i % this->stride;

		if (mod >= this->param_count)
			continue;

		if (! this->params_enabled[mod])
			continue;

		buffer[j++] = array[i];
	}

	this->data = (void *)buffer;
}

/**
 * Fill out this class with data from the given Accessor element.
 **/
void
load_from_accessor(domAccessorRef ref)
{
	size_t i;
	const char *pname;
	domParam_Array params = ref->getParam_array();

	this->stride = ref->getStride();
	this->source = ref->getSource().getElement();
	this->param_count = params.getCount();

	this->used_param_count = 0;

	for (i = 0; i < this->param_count; i++) {
		pname = params[i]->getName();

		if (pname && strlen(pname)) {
			used_param_count++;
			this->params_enabled.append(1);
		} else {
			this->params_enabled.append(0);
		}
	}


	this->set_gl_type();
}

/**
 * Fill out this class with data from the given Source element.
 **/
void
load_from_source(domSourceRef ref)
{
	domSource::domTechnique_commonRef tech =
		ref->getTechnique_common();

	if (! tech)
		errx(1, "COLLADA source without technique_common"
		     " unimplemented");

	this->load_from_accessor(tech->getAccessor());
}

public:

void *
copy_out(void *target, size_t idx)
{
	size_t span = this->used_param_count * this->datum_sz;
	char *src = (char *)this->data;
	char *dst = (char *)target;

	memcpy(dst, &src[span * idx], span);

	return (void *)&dst[span];
}

/**
 * Add this class to a vertex buffer format.
 **/
void
add_to_vbuf(vbuf_fmt_t *fmt)
{
	vbuf_fmt_add(fmt, this->name, this->used_param_count, this->gl_type);
}

/**
 * Construct this class from a COLLADA input.
 **/
DAEDataSource(domInputLocalOffsetRef input)
{
	domSourceRef source = input->getSource().getElement();
	char *name = xstrdup(input->getSemantic());
	char *c;
	domInputLocalRef vinput;

	if (! strcmp(name, "VERTEX")) {
		free(name);
		vinput = vert_to_pos(input);
		name = xstrdup(vinput->getSemantic());
		source = vinput->getSource().getElement();
	}

	for (c = name; *c; *c = tolower(*c), c++);

	if (source->typeID() != domSource::ID())
		errx(1, "COLLADA vertex input points to source that's"
		     " not a source");

	this->name = name;
	this->load_from_source(source);
}

/**
 * Destroy this class.
 **/
~DAEDataSource()
{
	free(this->name);
	free(this->data);
}

}; /* class DAEDataSource */

/**
 * Get a transform matrix to correct the up axis.
 **/
static void
dae_up_axis_to_xfrm(domUpAxisType up, float out[16])
{
	memset(out, 0, 16 * sizeof(float));

	out[0] = out[5] = out[10] = out[15] = 1;

	if (up == UPAXISTYPE_Z_UP) {
		out[5] = 0;
		out[6] = 1;
		out[9] = -1;
		out[10] = 0;
	} else if (up == UPAXISTYPE_X_UP) {
		out[0] = 0;
		out[1] = -1;
		out[4] = 1;
		out[5] = 0;
	}
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
	daeTArray<DAEDataSource *> sources;
	void *data;
	uint16_t *ebuf;
	void *loc;
	size_t j;
	mesh_t *out_mesh;
	GLenum type;
	object_t *ret;

	pa = mesh->getPolylist_array();
	if (! pa.getCount())
		return NULL;

	polylist = pa[0];

	inputs = polylist->getInput_array();
	input_count = inputs.getCount();

	input_stride = 0;
	for (i = 0; i < input_count; i++) {
		sources.append(new DAEDataSource(inputs[i]));
		sources[i]->add_to_vbuf(&fmt);
		
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
	/* FIXME: Might read in wrong order since we don't use this pop thing
	 */
	while (vbuf_fmt_pop_segment(&iter, NULL, &type, NULL, NULL)) {
		for (j = inputs[i]->getOffset(); j < vert_count * input_stride;
		     j += input_stride) {
			loc = sources[i]->copy_out(loc, indices[j]);
		}

		delete sources[i];

		i++;
	}

	ebuf = (uint16_t *)xcalloc(vert_count, sizeof(uint16_t));

	for (i = 0; i < vert_count; i++)
		ebuf[i] = i;

	out_mesh = mesh_create(vert_count, (float *)data, vert_count,
			       (uint16_t *)ebuf, fmt, GL_TRIANGLES);

	free(data);
	free(ebuf);

	ret = object_create(NULL);
	object_set_mesh(ret, out_mesh);
	mesh_ungrab(out_mesh);
	return ret;
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
 *
 * FIXME: Not sure if there's a specified order to apply transforms in. We kind
 * of just do what's convenient.
 **/
static void
dae_apply_transform(domNodeRef node, object_t *object, domUpAxisType up)
{
	daeTArray< daeSmartRef<daeElement> > elems = node->getChildren();
	domFloat4 fl;
	domFloat4x4 mat;
	quat_t quat;
	size_t i, j, k;
	MATRIX_DECL_IDENT(xfrm_total);
	float axis[16];
	float axis_trans[16];

	dae_up_axis_to_xfrm(up, axis);
	matrix_transpose(axis, axis_trans);

	for (i = 0; i < elems.getCount(); i++) {
		MATRIX_DECL_IDENT(xfrm);

		if (elems[i]->typeID() == domTranslate::ID()) {
			fl = ((domTranslateRef)elems[i])->getValue();
			xfrm[12] = fl[0];
			xfrm[13] = fl[1];
			xfrm[14] = fl[2];
		} else if (elems[i]->typeID() == domRotate::ID()) {
			fl = ((domRotateRef)elems[i])->getValue();
			fl[3] = fl[3] * 3.14159 / 180;
			quat_init(&quat, fl[0], fl[1], fl[2], fl[3]);
			quat_to_matrix(&quat, xfrm);
		} else if (elems[i]->typeID() == domScale::ID()) {
			fl = ((domScaleRef)elems[i])->getValue();
			xfrm[0] = fl[0];
			xfrm[5] = fl[1];
			xfrm[10] = fl[2];
		} else if (elems[i]->typeID() == domMatrix::ID()) {
			mat = ((domMatrixRef)elems[i])->getValue();
			for (k = 0; k < 4; k++)
				for (j = 0; j < 4; j++)
					xfrm[k * 4 + j] = mat[j * 4 + k];
		} else continue;

		matrix_multiply(xfrm_total, xfrm, xfrm_total);
	}

	matrix_multiply(axis_trans, xfrm_total, xfrm_total);
	matrix_multiply(xfrm_total, axis, xfrm_total);

	object->private_transform = (float *)xcalloc(16, sizeof(float));
	memcpy(object->private_transform, axis_trans, 16 * sizeof(float));

	object_apply_pretransform(object, xfrm_total);
}

/**
 * Given a domNode eleement to specify it, create a new object with the given
 * parent.
 **/
static object_t *
dae_process_node(domNodeRef node, object_t *parent, domUpAxisType up)
{
	domInstance_geometry_Array arr = node->getInstance_geometry_array();
	object_t *object;
	const char *name;
	domNode_Array children;
	size_t i;

	if (arr.getCount() > 1)
		errx(1, "COLLADA import supports exactly 1 mesh per node."
		     " Found %lu", arr.getCount());

	if (arr.getCount() == 0) {
		object = object_create(parent);

		if (parent)
			object_ungrab(object);

		goto out;
	}

	object = dae_load_geom(arr[0]->getUrl().getElement());

	if (! object)
		errx(1, "Unsupported mesh format or mesh missing");

	if (! parent)
		goto out;

	/* If we have no parent, then the importer itself is holding a
	 * reference. If we have a parent, then we add a new reference so that
	 * in either case we can destroy a reference after reparenting.
	 */
	if (object->parent)
		object_grab(object);

	object_reparent(object, parent);
	object_ungrab(object);

out:
	dae_apply_transform(node, object, up);
	name = node->getName();
	children = node->getNode_array();

	if (! name)
		name = node->getId();

	if (name)
		object_set_name(object, name);

	for (i = 0; i < children.getCount(); i++)
		dae_process_node(children[i], object, up);

	return object;
}

/**
 * Process COLLADA nodes from a visual scene.
 **/
static void
dae_process_scenes(domVisual_scene_Array &scenes,
		   daeTArray<object_t *> &output,
		   domUpAxisType up)
{
	domNode_Array nodes;
	size_t i;
	size_t j;
	object_t *obj;

	for (i = 0; i < scenes.getCount(); i++) {
		nodes = scenes[i]->getNode_array();

		for (j = 0; j < nodes.getCount(); j++) {
			obj = dae_process_node(nodes[j], NULL, up);
			if (obj)
				output.append(obj);
		}
	}
}

/**
 * Process COLLADA nodes from the visual scene library
 **/
static void
dae_get_nodes_scenes(domCOLLADA *doc, daeTArray<object_t *> &output,
		     domUpAxisType up)
{
	domLibrary_visual_scenes_Array lib;
	domVisual_scene_Array scenes;
	size_t i;
	size_t j;

	lib = doc->getLibrary_visual_scenes_array();

	for (i = 0; i < lib.getCount(); i++) {
		scenes = lib[i]->getVisual_scene_array();

		for (j = 0; j < scenes.getCount(); j++)
			dae_process_scenes(scenes, output, up);
	}
}

/**
 * Process COLLADA nodes from the node library
 **/
static void
dae_get_nodes_lib(domCOLLADA *doc, daeTArray<object_t *> &output,
		  domUpAxisType up)
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
			obj = dae_process_node(nodes[j], NULL, up);
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
	domAsset::domUp_axisRef up_elem = doc->getAsset()->getUp_axis();
	domUpAxisType up = up_elem ? up_elem->getValue() : UPAXISTYPE_Y_UP;
	object_t **ret;
	size_t i;

	dae_get_nodes_scenes(doc, output, up);
	dae_get_nodes_lib(doc, output, up);

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
EXPORT(dae_load);

} /* extern "C" */

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

#include <err.h>

#include "material.h"

/**
 * Create a new material.
 **/
material_t *
material_create(void)
{
	material_t *ret = xmalloc(sizeof(material_t));

	ret->shaders = NULL;
	ret->passes = 0;

	list_init(&ret->uniforms);

	return ret;
}

/**
 * Set the number of passes this material knows about.
 **/
static void
material_set_passes(material_t *material, size_t passes)
{
	size_t i;
	material->shaders = xrealloc(material->shaders,
				     passes * sizeof(shader_t *));

	for (i = material->passes; i < passes; i++)
		material->shaders[i] = NULL;

	material->passes = passes;
}

/**
 * Set the shader used for a given pass by this material.
 **/
void
material_set_pass_shader(material_t *material, size_t pass, shader_t *shader)
{

	if (pass >= material->passes)
		material_set_passes(material, pass + 1);

	material->shaders[pass] = shader;
}

/**
 * Set a uniform that this material should pass to the shader at draw time.
 **/
void
material_set_uniform(material_t *material, const char *name,
		     shader_uniform_type_t type, void *data)
{
	material_uniform_t *uni = xmalloc(sizeof(material_uniform_t));

	uni->name = xstrdup(name);
	uni->type = type;
	uni->data = data;

	list_init(&uni->link);
	list_insert(&material->uniforms, &uni->link);
}

/**
 * Tell a shader about our uniform.
 **/
static void
material_uniform_load(material_uniform_t *uniform, shader_t *shader)
{
	switch(uniform->type) {
	case SHADER_UNIFORM_MAT4:
		shader_set_uniform_mat(shader, uniform->name, uniform->data);
		break;
	case SHADER_UNIFORM_SAMP2D:
		shader_set_uniform_samp2D(shader, uniform->name, uniform->data);
		break;
	default:
		errx(1, "Invalid uniform type %d", uniform->type);
	};
}

/**
 * Prepare to draw something with the given material.
 **/
int
material_activate(material_t *material, size_t pass)
{
	material_uniform_t *uni;

	if (pass >= material->passes)
		return 0;

	if (! material->shaders[pass])
		return 0;

	shader_activate(material->shaders[pass]);

	foreach(&material->uniforms) {
		uni = (material_uniform_t *)pos;

		material_uniform_load(uni, material->shaders[pass]);
	}

	return 1;
}

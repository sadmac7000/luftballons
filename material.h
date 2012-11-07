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

#ifndef MATERIAL_H
#define MATERIAL_H

#include "util.h"
#include "shader.h"

/**
 * A uniform that this material sets for shaders.
 *
 * link: List node for the material to keep track of this object
 * name: The in-shader name for the uniform
 * type: The data type for the uniform
 * value: The value we're setting to
 **/
typedef struct material_uniform {
	list_node_t link;
	const char *name;
	shader_uniform_type_t type;
	void *data;
} material_uniform_t;

/**
 * A material. Dictates a set of uniforms and a shader for each pass.
 *
 * shaders: An array of shaders with index corresponding to pass
 * passes: The number of items in `shaders`
 * uniforms: A list of uniforms
 **/
typedef struct material {
	shader_t **shaders;
	size_t passes;
	list_head_t uniforms;
} material_t;

#ifdef __cplusplus
extern "C" {
#endif

material_t *material_create(void);
void material_set_pass_shader(material_t *material, size_t pass,
			      shader_t *shader);
void material_set_uniform(material_t *material, const char *name,
			  shader_uniform_type_t type, void *data);
int material_activate(material_t *material, size_t pass);

#ifdef __cplusplus
}
#endif

#endif /* MATERIAL_H */

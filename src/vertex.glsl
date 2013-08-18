#version 130

/**
 * Copyright © 2013 Casey Dahlin
 *
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

in vec4 position;
in vec4 color;
in vec3 normal;
in vec4 texcoord;
out vec4 colorout;
out vec4 normalout;
out vec4 texcoordout;
out vec4 posout;
uniform mat4 transform;
uniform mat4 clip_transform;

void main()
{
	posout = transform * position;
	gl_Position = clip_transform * transform * position;
	colorout = color;
	texcoordout = texcoord;
	normalout = transform * vec4(normal, 0);
}

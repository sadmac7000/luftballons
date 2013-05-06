#version 120

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

varying vec4 colorout;
varying vec4 texcoordout;
varying vec4 normalout;
uniform sampler2D diffusemap;

vec4 camdir = vec4(0,0,1, 0);

void main()
{
	float cos_angle = clamp(dot(normalize(normalout), normalize(camdir)), 0, 1);
	vec4 color = texture2D(diffusemap, texcoordout.st);
	gl_FragColor =  vec4(color.rgb * cos_angle, color.a);
}

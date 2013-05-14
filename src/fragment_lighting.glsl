#version 130

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

in vec4 posout;
uniform sampler2D normal_buf;
uniform sampler2D position_buf;
uniform sampler2D diffuse_buf;

vec4 camdir = vec4(0,0,1, 0);

void main()
{
	vec2 pos = (posout.xy + vec2(1, 1)) / 2;
	vec4 normal = texture2D(normal_buf, pos);
	vec4 position = texture2D(position_buf, pos);
	vec4 diffuse = texture2D(diffuse_buf, pos);

	float cos_angle = clamp(dot(normalize(normal), normalize(camdir)), 0, 1);
	gl_FragColor = vec4(diffuse.rgb * cos_angle, diffuse.a);
}

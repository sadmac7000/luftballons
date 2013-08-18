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

in vec4 posout;
uniform sampler2D normal_buf;
uniform sampler2D position_buf;
uniform sampler2D diffuse_buf;
uniform mat4 transform;
uniform vec4 light_color;

void main()
{
	vec2 screen_pos = (posout.xy + vec2(1, 1)) / 2;
	vec4 normal = normalize(texture2D(normal_buf, screen_pos));
	vec4 position = texture2D(position_buf, screen_pos);
	vec4 diffuse = texture2D(diffuse_buf, screen_pos);
	vec4 light_pos = transform * vec4(0,0,0,1);
	vec3 to_light = normalize(light_pos.xyz - position.xyz);

	float cos_angle = clamp(dot(normal.xyz, to_light.xyz), 0, 1);
	gl_FragColor = vec4(diffuse.rgb * cos_angle, diffuse.a) * light_color;
}

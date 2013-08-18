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

in vec4 colorout;
in vec4 posout;
in vec4 texcoordout;
in vec4 normalout;
uniform sampler2D last_depth;
uniform int last_depth_valid;

void main()
{
	vec2 screen_pos = vec2(gl_FragCoord.x / 800, gl_FragCoord.y / 600);
	float depth = texture2D(last_depth, screen_pos).r;
	if (last_depth_valid != 0 && depth >= gl_FragCoord.z)
		discard;
	gl_FragData[0] = normalout;
	gl_FragData[1] = posout;
	gl_FragData[2] = colorout;
}

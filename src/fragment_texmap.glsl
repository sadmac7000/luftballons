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

in vec4 colorout;
in vec4 texcoordout;
in vec4 normalout;
in vec4 posout;
uniform sampler2D diffusemap;

void main()
{
	gl_FragData[0] =  normalout;
	gl_FragData[1] =  posout;
	gl_FragData[2] =  texture2D(diffusemap, texcoordout.st);
}

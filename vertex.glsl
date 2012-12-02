#version 120

attribute vec4 position;
attribute vec4 color;
attribute vec4 texcoord;
varying vec4 colorout;
varying vec4 texcoordout;
uniform mat4 transform;

void main()
{
	gl_Position = transform * position;
	colorout = color;
	texcoordout = texcoord;
}

#version 120

attribute vec4 position;
attribute vec4 color;
varying vec4 colorout;
uniform mat4 transform;

void main()
{
	gl_Position = transform * position;
	colorout = color;
}

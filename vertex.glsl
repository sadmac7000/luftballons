#version 120

attribute vec4 position;
attribute vec4 colorin;
varying vec4 colorout;
uniform vec4 offset;
uniform mat4 transform;

void main()
{
	gl_Position = transform * (position + offset);
	colorout = colorin;
}

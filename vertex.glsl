#version 120

attribute vec4 position;
attribute vec4 colorin;
varying vec4 colorout;
uniform mat4 transform;

void main()
{
	gl_Position = position * transform;
	colorout = colorin;
}

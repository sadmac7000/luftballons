#version 120

attribute vec4 position;
varying vec4 colorout;

void main()
{
	gl_Position = position;
	colorout = position;
}

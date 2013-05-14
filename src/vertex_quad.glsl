#version 120

attribute vec4 position;
varying vec4 posout;

void main()
{
	gl_Position = posout = position;
}

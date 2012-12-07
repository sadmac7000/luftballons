#version 120

attribute vec4 position;
attribute vec4 color;
attribute vec4 normal;
attribute vec4 texcoord;
varying vec4 colorout;
varying vec4 normalout;
varying vec4 texcoordout;
uniform mat4 transform;
uniform mat4 camera_transform;

void main()
{
	gl_Position = transform * position;
	colorout = color;
	texcoordout = texcoord;
	normalout = normalize(camera_transform * normal);
}

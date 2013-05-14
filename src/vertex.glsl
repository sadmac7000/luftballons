#version 120

attribute vec4 position;
attribute vec4 color;
attribute vec4 normal;
attribute vec4 texcoord;
varying vec4 colorout;
varying vec4 normalout;
varying vec4 texcoordout;
varying vec4 posout;
uniform mat4 transform;
uniform mat4 normal_transform;
uniform mat4 clip_transform;

void main()
{
	gl_Position = posout = clip_transform * transform * position;
	colorout = color;
	texcoordout = texcoord;
	normalout = normalize(normal_transform * normal);
}

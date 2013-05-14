#version 130

in vec4 position;
in vec4 color;
in vec4 normal;
in vec4 texcoord;
out vec4 colorout;
out vec4 normalout;
out vec4 texcoordout;
out vec4 posout;
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

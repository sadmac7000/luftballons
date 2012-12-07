#version 120

varying vec4 colorout;
varying vec4 texcoordout;
varying vec4 normalout;
uniform sampler2D diffusemap;

vec4 camdir = vec4(0,0,1, 0);

void main()
{
	float cos_angle = clamp(dot(normalize(normalout), normalize(camdir)), 0, 1);
	gl_FragColor = texture2D(diffusemap, texcoordout.st) * cos_angle;
}

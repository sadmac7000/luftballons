#version 120

varying vec4 colorout;
varying vec4 texcoordout;
uniform sampler2D diffusemap;

void main()
{
	gl_FragColor = colorout + texture2D(diffusemap, texcoordout.st);
}

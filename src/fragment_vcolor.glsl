#version 120

varying vec4 colorout;
varying vec4 texcoordout;
varying vec4 normalout;

void main()
{
	gl_FragColor = colorout;
}

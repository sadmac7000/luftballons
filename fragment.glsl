#version 120

varying vec4 colorout;

void main()
{
	gl_FragColor = vec4(1.0f, 0.0f, 1.0f, 1.0f) + colorout;
}

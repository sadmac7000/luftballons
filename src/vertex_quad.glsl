#version 130

in vec4 position;
out vec4 posout;

void main()
{
	gl_Position = posout = position;
}

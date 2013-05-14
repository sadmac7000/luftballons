#version 130

in vec4 posout;
uniform sampler2D normal_buf;
uniform sampler2D position_buf;
uniform sampler2D diffuse_buf;

vec4 camdir = vec4(0,0,1, 0);

void main()
{
	vec2 pos = (posout.xy + vec2(1, 1)) / 2;
	vec4 normal = texture2D(normal_buf, pos);
	vec4 position = texture2D(position_buf, pos);
	vec4 diffuse = texture2D(diffuse_buf, pos);

	float cos_angle = clamp(dot(normalize(normal), normalize(camdir)), 0, 1);
	gl_FragColor = vec4(diffuse.rgb * cos_angle, diffuse.a);
}

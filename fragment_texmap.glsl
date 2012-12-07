#version 120

varying vec4 colorout;
varying vec4 texcoordout;
varying vec4 normalout;
uniform sampler2D diffusemap;

vec4 campos = vec4(0,0,1, 1);

void bdrf(vec4 fragpos, vec4 lightpos, vec4 normal)
{
	vec4 to_light = lightpos - fragpos;
	vec4 to_cam = campos - fragpos;
	vec4 halfway = (to_light - to_cam) / 2 + to_cam;
	float t = dot(halfway, normal);
}

void main()
{
	float cos_angle = clamp(dot(normalize(campos), normalize(normalout)), 0, 1);
	gl_FragColor = texture2D(diffusemap, texcoordout.st) * cos_angle;
}

#version 440 core

layout(location = 0) out vec4 color;

in vec4 FragPos;

uniform mat2 uSigma; //sigma matrix
uniform vec4 uColor; //sigma matrix

float p(vec2 x)
{
	vec2 spos = x * uSigma;
	return exp(-0.5 * dot(spos, x));
}

void main()
{
	float c = p(FragPos.xy);

    color = vec4(c*uColor.x, c*uColor.y, c*uColor.z, c*uColor.w);
	if(c < 0.01) color = vec4(uColor.x, uColor.y, uColor.z, uColor.w); //color = vec4(uColor.x, uColor.y, uColor.z, uColor.w);
}
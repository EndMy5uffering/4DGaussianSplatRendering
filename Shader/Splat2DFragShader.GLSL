#version 440 core

layout(location = 0) out vec4 color;

in vec4 FragPos;
in vec2 Scale;

in vec4 SplatPos;

uniform mat2 uSigma; //sigma matrix
uniform vec4 uColor; //sigma matrix
uniform float uN;

float p(vec2 x, vec2 mu)
{
	vec2 pos = (x-mu);
	vec2 spos = pos * uSigma;
	return exp(-0.5 * dot(spos, pos));
}

float p2(vec4 splatcenter)
{
	return exp(-0.5*length(splatcenter.xyz - FragPos.xyz));
}

void main()
{
	float c = p(FragPos.xy, SplatPos.xy);
	//float c = p2(SplatPos);
	//c = 1.0f;
    color = vec4(c*uColor.x, c*uColor.y, c*uColor.z, c);
	if(c < 0.01) discard;
}
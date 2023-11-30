#version 440 core

layout(location = 0) out vec4 color;

in vec4 FragPos;
in vec2 Scale;

in vec4 SplatPos;

uniform mat2 uSigma; //sigma matrix
uniform vec4 uColor; //sigma matrix

float p(vec2 x, vec2 mu)
{
	return exp(-0.5 * dot(((x-mu)*uSigma), (x-mu)));
}

float p2(vec4 fpos, vec4 splatcenter)
{
	return 1/exp(-0.5*length(splatcenter.xyz - fpos.xyz));
}

void main()
{
	//float c = p(vec2(FragPos.x * Scale.x, FragPos.y * Scale.y), SplatPos.xy);
	float c = p2(FragPos, SplatPos);

    color = vec4(c*uColor.x, c*uColor.y, c*uColor.z, c);
	if(c < 0.01) color = vec4(uColor.x, uColor.y, uColor.z, 1.0);
}
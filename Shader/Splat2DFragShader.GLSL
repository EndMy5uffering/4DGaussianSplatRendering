#version 440 core

layout(location = 0) out vec4 color;

in vec4 FragPos;
in vec2 Scale;

uniform mat2 uSigma; //sigma matrix

float p(vec2 x, vec2 mu)
{
	return exp(-0.5 * dot(((x-mu)*uSigma), (x-mu)));
}

void main()
{
	float c = p(vec2(FragPos.x * Scale.x, FragPos.y * Scale.y), vec2(0,0));

    color = vec4(c, c*0.0, c*0.0, c);
	if(c < 0.01) color = vec4(1.0, 0.0, 0.0, 1.0);
}
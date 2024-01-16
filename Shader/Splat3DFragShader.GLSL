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

    //color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    color = vec4(c*uColor.x, c*uColor.y, c*uColor.z, c);;//vec4(c*uColor.x, c*uColor.y, c*uColor.z, c);
	if(c < 0.01) color = vec4(0.0, 0.0, 0.0, 1.0);
}
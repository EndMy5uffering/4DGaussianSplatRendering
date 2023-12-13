#version 440 core

layout(location = 0) out vec4 color;

in vec4 frag;

int mat2 Sigma;

//uniform mat2 uSigma; //sigma matrix
uniform vec4 uColor; //sigma matrix

float p()
{
	vec2 pos = frag.xy;
	vec2 spos = pos * Sigma;
	return exp(-0.5 * dot(spos, pos));
}

void main()
{
	float c = p();

    color = vec4(c*uColor.x, c*uColor.y, c*uColor.z, c);
	if(c < 0.01) discard;
}
#version 440 core

layout(location = 0) out vec4 color;

in vec4 oFragPos;
in mat2 oSig;
in vec4 oColor;
in vec2 oSSPos;

float p(vec2 x)
{
	vec2 spos = oSig * x;
	return exp(-0.5 * dot(spos, x));
}

void main()
{
	vec2 offpos = oFragPos.xy - oSSPos;
	float c = p(oFragPos.xy);

    //color = vec4(c*oColor.x, c*oColor.y, c*oColor.z, c*oColor.w);
    //color = c * oColor;
    color = vec4(oColor.xyz, c * oColor.w);
	if(c < 0.0001) discard;// color = vec4(oColor.x, oColor.y, oColor.z, 1.0);
}
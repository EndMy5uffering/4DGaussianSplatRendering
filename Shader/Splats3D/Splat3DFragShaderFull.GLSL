#version 440 core

layout(location = 0) out vec4 color;

in vec4 FragPos;
in mat2 oSig;
in vec4 oColor;

float p(vec2 x)
{
	vec2 spos = x * oSig;
	return exp(-0.5 * dot(spos, x));
}

void main()
{
	float c = p(FragPos.xy);

    //color = vec4(c*oColor.x, c*oColor.y, c*oColor.z, c*oColor.w);
    color = vec4(c*oColor.x, c*oColor.y, c*oColor.z, c*oColor.w);
	if(c < 0.01) discard; //color = vec4(oColor.x, oColor.y, oColor.z, oColor.w);; //color = vec4(uColor.x, uColor.y, uColor.z, uColor.w);
}
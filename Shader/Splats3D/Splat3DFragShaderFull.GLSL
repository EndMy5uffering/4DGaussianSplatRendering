#version 440 core

layout(location = 0) out vec4 color;

in vec4 oFragPos;
in mat2 oSig;
in vec4 oColor;
in float oFaulty;

float p(vec2 x)
{
	vec2 spos = x * oSig;
	return exp(-0.5 * dot(spos, x));
}

void main()
{
	if(oFaulty > 0.0) discard;
	float c = p(oFragPos.xy);

    //color = vec4(c*oColor.x, c*oColor.y, c*oColor.z, c*oColor.w);
    color = vec4(c*oColor.x, c*oColor.y, c*oColor.z, c*oColor.w);
	if(c < 0.0001) discard; //color = vec4(oColor.x, oColor.y, oColor.z, 1.0); //color = vec4(oColor.x, oColor.y, oColor.z, oColor.w);; //color = vec4(uColor.x, uColor.y, uColor.z, uColor.w);
}
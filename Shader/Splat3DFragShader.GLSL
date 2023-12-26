#version 440 core

layout(location = 0) out vec4 color;

void main()
{
    color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    //color = vec4(c*uColor.x, c*uColor.y, c*uColor.z, c);
	//if(c < 0.01) discard;
}
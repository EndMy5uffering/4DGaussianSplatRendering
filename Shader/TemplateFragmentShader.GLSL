#version 440 core

layout(location = 0) out vec4 color;
//uniform float time;
//uniform vec4 u_color;

in vec2 texCoord;

uniform sampler2D u_tex0;

void main()
{
    //color = vec4(0.5f, 0.0f, 0.0f, 1.0f);
    //color = u_color;


    color = texture(u_tex0, texCoord);
	if(color.a < 0.01) discard;
}
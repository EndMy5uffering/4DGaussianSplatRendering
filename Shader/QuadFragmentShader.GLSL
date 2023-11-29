#version 440 core

layout(location = 0) out vec4 color;

in vec2 texCoord;

uniform sampler2D u_tex0;

void main()
{
    color = texture(u_tex0, texCoord);
}
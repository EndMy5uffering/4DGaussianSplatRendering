#version 440 core

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 texPos;
//uniform float time;
uniform mat4 u_modle;
uniform mat4 u_cam;

out vec2 texCoord;

void main()
{
    //gl_Position = u_view * inPos;

    gl_Position = u_cam * u_modle * inPos;
    texCoord = texPos;
}
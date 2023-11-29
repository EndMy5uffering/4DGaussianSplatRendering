#version 440 core

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 texPos;
uniform mat4 u_modle;
uniform mat4 u_cam;
uniform vec4 u_normal;

out DATA
{
    vec3 Normal;
    vec2 uvCords;
    mat4 projection;
} data_out;

void main()
{
    gl_Position = u_cam * u_modle * inPos;
    data_out.Normal = u_normal;
    data_out.uvCords = texPos;
    data_out.projection = u_cam * u_modle;
}
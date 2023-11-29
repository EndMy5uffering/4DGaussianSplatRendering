#version 440 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out vec3 normal;
out vec3 color;
out vec2 uvCords;

in DATA
{
    vec3 Normal;
    vec3 color;
    vec2 uvCords;
    mat4 projection;
} data_in[];

void main()
{
    
}
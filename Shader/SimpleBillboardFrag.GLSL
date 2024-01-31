#version 460

out vec4 FragOutput;
in vec4 oColor;

void main()
{
    FragOutput = oColor;
}
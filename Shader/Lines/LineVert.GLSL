#version 450 core

layout (location = 0) in vec3 aPos;
uniform mat4 uViewProj;

out vec4 aColor;
uniform vec4 uColor;

void main() {
    aColor = uColor;
    gl_Position = uViewProj * vec4(aPos, 1.0);
}
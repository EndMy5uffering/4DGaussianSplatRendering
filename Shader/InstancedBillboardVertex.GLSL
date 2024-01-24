#version 450 core

layout (location = 0) in vec2 aPos;

out vec4 aColor;
uniform vec4 uColor;

void main() {
    aColor = uColor;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
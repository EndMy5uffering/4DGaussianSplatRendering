#version 450 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

out vec4 oColor;

uniform mat4 uViewProject;

void main() {
    gl_Position = uViewProject * vec4(aPos, 0.0, 1.0);
    //gl_Position = gl_Position + vec4(aScreenPos.x, aScreenPos.y, 0.0f, 0.0f);
    oColor = aColor;
}
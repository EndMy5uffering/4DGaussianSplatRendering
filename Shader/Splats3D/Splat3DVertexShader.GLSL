/*
    Old vertex shader used for billboard
*/
#version 440 core

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 texPos;

uniform vec2 uScale;
uniform vec2 uVec1;
uniform vec2 uVec2;
uniform vec2 uScreenPos;
uniform mat4 uProj;

out vec4 FragPos;

mat4 mScale = mat4(
     uScale.x*3,      0.0, 0.0, 0.0,
         0.0,  uScale.y*3, 0.0, 0.0,
         0.0,      0.0, 1.0, 0.0,
         0.0,      0.0, 0.0, 1.0
);

mat4 mRot = mat4(
    uVec1.x, uVec2.x, 0.0, 0.0,
    uVec1.y, uVec2.y, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
);

void main()
{
    gl_Position = mRot * mScale * inPos;
    FragPos = gl_Position;
    gl_Position = gl_Position + vec4(uScreenPos.x, uScreenPos.y, 0.0f, 0.0f);
}
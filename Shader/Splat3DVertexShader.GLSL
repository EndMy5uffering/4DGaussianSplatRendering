#version 440 core

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 texPos;

uniform vec2 uScale;
uniform vec2 uVec1;
uniform vec2 uVec2;
uniform mat4 uProj;
//uniform mat4 uViewProj;

mat4 mScale = mat4(
    uScale.x*10,      0.0, 0.0, 0.0,
         0.0, uScale.y*10, 0.0, 0.0,
         0.0,      0.0, 1.0, 0.0,
         0.0,      0.0, 0.0, 1.0
);

mat4 mRot = mat4(
    uVec1.x, uVec2.x, 0.0, 0.0,
    uVec1.y, uVec2.y, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
);

mat4 mPosRotScal = mRot * mScale;

mat4 offset = mat4(
    0.1, 0.0, 0.0, 0.0,
    0.0, 0.1, 0.0, 0.0,
    0.0, 0.0, 0.1, 0.0,
    0.0, 0.0, -1.0, 1.0
);

void main()
{
    gl_Position = uProj * offset * mPosRotScal * inPos;
}
#version 440 core

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 texPos;

uniform mat3 uJ;
uniform mat3 uW;

out mat2 Sigma;

void main()
{
    mat4 mvMatrix = uView * mPosRotScal;

    mvMatrix[0][0] = mPosRotScal[0][0];
    mvMatrix[0][1] = mPosRotScal[0][1];
    mvMatrix[0][2] = mPosRotScal[0][2];

    mvMatrix[1][0] = mPosRotScal[1][0];
    mvMatrix[1][1] = mPosRotScal[1][1];
    mvMatrix[1][2] = mPosRotScal[1][2];
    
    mvMatrix[2][0] = mPosRotScal[2][0];
    mvMatrix[2][1] = mPosRotScal[2][1];
    mvMatrix[2][2] = mPosRotScal[2][2];

    gl_Position = uProj * mvMatrix * inPos;
}
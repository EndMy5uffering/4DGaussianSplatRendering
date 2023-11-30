#version 440 core

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 texPos;

uniform mat4 uModle;
uniform mat4 uProj;
uniform mat4 uView;
uniform vec2 uScale;
uniform vec4 uSplatPos;
uniform mat4 uRot;

out vec4 FragPos;
out vec2 Scale;
out vec4 SplatPos;

mat4 mScale = mat4(
    uScale.x,      0.0, 0.0, 0.0,
         0.0, uScale.y, 0.0, 0.0,
         0.0,      0.0, 1.0, 0.0,
         0.0,      0.0, 0.0, 1.0
);

mat4 mRotScal = uRot * mScale;

void main()
{
    mat4 mvMatrix = uView * uModle;

    mvMatrix[0][0] = mRotScal[0][0];
    mvMatrix[0][1] = mRotScal[0][1];
    mvMatrix[0][2] = mRotScal[0][2];

    mvMatrix[1][0] = mRotScal[1][0];
    mvMatrix[1][1] = mRotScal[1][1];
    mvMatrix[1][2] = mRotScal[1][2];
    
    mvMatrix[2][0] = mRotScal[2][0];
    mvMatrix[2][1] = mRotScal[2][1];
    mvMatrix[2][2] = mRotScal[2][2];

    gl_Position = uProj * mvMatrix * inPos;
    FragPos = mRotScal * uModle * vec4(inPos.xyz, 1.0);
    Scale = uScale;
    SplatPos = vec4(uSplatPos.xyz, 1.0);
}
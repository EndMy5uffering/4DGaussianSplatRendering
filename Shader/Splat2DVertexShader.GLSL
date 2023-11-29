#version 440 core

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 texPos;

uniform mat4 uModle;
uniform mat4 uProj;
uniform mat4 uView;
uniform vec2 uScale;

out vec2 texCoord;
out vec4 FragPos;
out vec2 Scale;

mat4 mScale = mat4(
    uScale.x,      0.0, 0.0, 0.0,
         0.0, uScale.y, 0.0, 0.0,
         0.0,      0.0, 1.0, 0.0,
         0.0,      0.0, 0.0, 1.0
);

uniform float uRot;
float ang = 3.14159265 * 90.0 / 180.0;

mat4 mRot = mat4(
    cos(uRot), -sin(uRot), 0.0, 0.0,
    sin(uRot),  cos(uRot), 0.0, 0.0,
         0.0,       0.0, 1.0, 0.0,
         0.0,       0.0, 0.0, 1.0
);

mat4 mRotScal = mRot * mScale;

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
    FragPos = mRot * uModle * vec4(inPos.xyz, 1.0);
    texCoord = texPos;
    Scale = uScale;
}
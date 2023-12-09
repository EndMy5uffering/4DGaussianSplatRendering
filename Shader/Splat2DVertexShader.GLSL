#version 440 core

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 texPos;

//uniform mat4 uModle;
uniform mat4 uProj;
uniform mat4 uView;
uniform vec2 uScale;
uniform vec4 uSplatPos;
uniform vec2 uVec1;
uniform vec2 uVec2;

out vec4 FragPos;
out vec2 Scale;
out vec4 SplatPos;

mat4 mScale = mat4(
    uScale.x*3,      0.0, 0.0, 0.0,
         0.0, uScale.y*3, 0.0, 0.0,
         0.0,      0.0, 1.0, 0.0,
         0.0,      0.0, 0.0, 1.0
);

mat4 mRot = mat4(
    uVec1.x, uVec1.y, 0.0, 0.0,
    uVec2.x, uVec2.y, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
);

mat4 splatPosMat = mat4(
    1.0, 0.0, 0.0, uSplatPos.x,
    0.0, 1.0, 0.0, uSplatPos.y,
    0.0, 0.0, 1.0, uSplatPos.z,
    0.0, 0.0, 0.0, uSplatPos.w
);

mat4 mPosRotScal = splatPosMat * mRot * mScale;

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
    FragPos = mPosRotScal * inPos; /* mRotScal * inPos */
    Scale = uScale;
    SplatPos = vec4(uSplatPos.xyz, 1.0);
}
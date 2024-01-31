#version 440 core

layout(location = 0) in vec2 iVPos;
layout(location = 1) in vec4 iSPos;
layout(location = 2) in vec4 iScale;
layout(location = 3) in mat4 iSig;
layout(location = 4) in vec4 iColor;


void main()
{
    gl_Position = mRot * mScale * inPos;
    FragPos = gl_Position;
    gl_Position = gl_Position + vec4(uScreenPos.x, uScreenPos.y, 0.0f, 0.0f);
}

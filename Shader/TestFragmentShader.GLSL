#version 330 core

layout(location = 0) out vec4 color;
layout(origin_upper_left) in vec4 gl_FragCoord;

//in vec2 gl_PointCoord;

void main()
{
    color = vec4(gl_FragCoord.x/640.0f, gl_FragCoord.y/480.0f, 0.0f, 1.0f);
    //color = vec4(gl_PointCoord.xy, 0.0f, 1.0f);
}
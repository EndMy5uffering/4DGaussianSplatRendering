#version 440 core

//#define M_PI 3.1415926535897932384626433832795

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D texture; //Output texture

uniform vec4 uNormal; //normal vector on the plane
uniform mat3 uSigmainv; //sigma matrix

float evaluate(vec3 x)
{
	return exp((x*uSigmainv*x));
}

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	//vec2 tex_size = imageSize(texture);

	float c = evaluate(vec3(pixel_coords.x, pixel_coords.y, 0));

	vec4 color = vec4(c, c, c, 1.0f);
	imageStore(texture, pixel_coords, color);
}

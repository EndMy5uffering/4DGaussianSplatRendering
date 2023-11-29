#version 440 core

#define M_PI 3.1415926535897932384626433832795

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 1) uniform image2D screem;


void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	
	vec2 tex_size = imageSize(screem);

	float normX = pixel_coords.x / tex_size.x;
	float normY = pixel_coords.y / tex_size.y;

	float sx = 0.5 + (sin(normX * 2 * M_PI) * 0.5);
	float sy = 0.5 + (cos(normY * 16 * M_PI) * 0.5);

	float c = sx*sy;
	vec4 color = vec4(c, c, c, 1.0f);
	imageStore(screem, pixel_coords, color);
}

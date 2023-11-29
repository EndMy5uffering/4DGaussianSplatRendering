#version 440 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D texture; //Output texture

uniform mat2 uSigma; //sigma matrix

float p(vec2 x, vec2 mu)
{
	return exp(-0.5 * dot(((x-mu)*uSigma), (x-mu)));
}

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	vec2 tex_size = 0.5 * imageSize(texture);

	float c = p(vec2(pixel_coords.x, pixel_coords.y), tex_size);

	vec4 color = vec4(c, c, c, c);

	imageStore(texture, pixel_coords, color);
}

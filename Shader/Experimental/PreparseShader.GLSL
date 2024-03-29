#version 440 core

struct RawData
{
  vec4 iSPos;
  mat4 iSig;
};

struct ProjtData
{
  vec4 iSPos;
  vec4 iSig;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0, std140) buffer InstanceData {
    RawData data[];
};

layout (binding = 1, std140) buffer ProjectedData {
    ProjtData data[];
};

float pTime(float t, float mu)
{
    return exp(-0.5 * (t - mu) * (1.0 / iData.iSig[3][3]) * (t - mu));
}

void main()
{
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	vec2 tex_size = 0.5 * imageSize(texture);

	float c = p(vec2(pixel_coords.x, pixel_coords.y), tex_size);

	vec4 color = vec4(c, c, c, c);

	imageStore(texture, pixel_coords, color);
}

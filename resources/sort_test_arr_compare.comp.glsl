#version 440

#define THREAD_IDX        gl_LocalInvocationIndex
#define THREADS_NUM       64
#define THREAD_BLOCK_IDX  (gl_WorkGroupID.x + gl_NumWorkGroups.x * (gl_WorkGroupID.y + gl_NumWorkGroups.z * gl_WorkGroupID.z))
#define THREAD_BLOCKS_NUM (gl_NumWorkGroups.x * gl_NumWorkGroups.y * gl_NumWorkGroups.z)
#define ITEMS_NUM         4

layout(local_size_x = THREADS_NUM, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer ssbo_arr_1 { uint b_arr_1[]; };
layout(std430, binding = 1) buffer ssbo_arr_2 { uint b_arr_2[]; };

layout(binding = 2) uniform atomic_uint ac_errors;
layout(std430, binding = 3) buffer ssbo_error_marks { uint b_error_marks[]; };

uniform uint u_arr_len;

uint to_key_idx(uint item_idx, uint thread_idx, uint thread_block_idx)
{
    return (thread_block_idx * ITEMS_NUM * THREADS_NUM) + (thread_idx * ITEMS_NUM) + item_idx;
}

void main()
{
    for (uint item_idx = 0; item_idx < ITEMS_NUM; item_idx++)
    {
        uint key_idx = to_key_idx(item_idx, THREAD_IDX, THREAD_BLOCK_IDX);
        if (key_idx < u_arr_len)
        {
            if (b_arr_1[key_idx] != b_arr_2[key_idx])
            {
                atomicCounterIncrement(ac_errors);
                b_error_marks[key_idx] = 1;
            }
        }
    }
}

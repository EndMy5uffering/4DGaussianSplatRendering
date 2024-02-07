#pragma once

#include <array>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "Renderer.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "GLFW/glfw3.h"
#include "GLEW/glew.h"



namespace Geometry 
{

    struct Vertex {
        glm::vec2 Position;
        glm::vec4 Color;
    };

    struct Splat4DVertex {
        glm::vec2 VPosition;
        glm::vec4 SPosition;
        glm::vec4 Color;
        glm::mat4 GeoInfo;
    };

    struct Splat3DVertex {
        glm::vec2 VPosition;
        glm::vec3 SPosition;
        glm::vec4 Color;
        glm::mat3 GeoInfo;
    };

    namespace BillboardVertexData
    {
        const float BillboardVertexBuff[] = {
            //     pos          |    uv    |
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // 0 0
            -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, // 1 1
             1.0f,  1.0f, 0.0f, 1.0f, 0.0f, // 2 2
             1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 3 3
        };

        const unsigned int BoxIdxBuff[] = {
            0, 1, 2,
            2, 1, 3,
        };

    }

    namespace Billboard2DVertexData
    {
        const float BillboardVertexBuff[] = {
            //   pos    |    uv     |
            -1.0f,  1.0f, 0.0f, 1.0f, // 0 0
            -1.0f, -1.0f, 1.0f, 1.0f, // 1 1
             1.0f,  1.0f, 1.0f, 0.0f, // 2 2
             1.0f, -1.0f, 0.0f, 0.0f, // 3 3
        };

        const unsigned int BoxIdxBuff[] = {
            0, 1, 2,
            2, 1, 3,
        };

    }

    class Billboard {

    public:

        Billboard(float* instancedata = 0, int instancedatasize = -1) :
            mVertexBuffer{ BillboardVertexData::BillboardVertexBuff, 5 * 4 * sizeof(float) },
            mIndexBuffer{ BillboardVertexData::BoxIdxBuff, 6 },
            mTransform(glm::mat4(1.0f))
        {
            VertexBufferLayout layout;
            layout.Push<float>(3);
            layout.Push<float>(2);
            mVertexArray.AddBuffer(mVertexBuffer, layout);

        }

        Billboard(glm::vec3 pos) : Billboard() { SetPosition(pos); }

        Billboard(glm::vec3 pos, glm::vec3 scale) : Billboard(pos) { SetScale(scale); }

        Billboard(glm::vec3 pos, float angle_rad, glm::vec3 axis) : Billboard(pos) { SetRotation(angle_rad, axis); }

        ~Billboard() { }

        void SetPosition(const glm::vec3& pos) { mTransform = glm::translate(mTransform, pos); }
        void SetRotation(float angle_rad, const glm::vec3& axis) { mTransform = glm::rotate(mTransform, angle_rad, axis); }
        void SetScale(const glm::vec3& scale) { mTransform = glm::scale(mTransform, scale); }
        glm::mat4 GetTransform() { return glm::mat4(mTransform); }
        void SetTransform(const glm::mat4& mat) { this->mTransform = glm::mat4(mat); }
        void Render(Renderer& renderer)
        {
            renderer.Draw(mVertexArray, mIndexBuffer);
        }

    private:
        VertexArray mVertexArray;
        IndexBuffer mIndexBuffer;
        VertexBuffer mVertexBuffer;
        glm::mat4 mTransform;
    };

}
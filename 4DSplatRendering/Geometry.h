#pragma once

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

    namespace BoxVertexData 
    {
        const float BoxVertexBuff[] = {
            //     pos          |    uv    |
            // FRONT
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // 0 0
            -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, // 1 1
             0.5f, -0.5f,  0.5f, 1.0f, 0.0f, // 2 2
             0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // 3 3
             // LEFT
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // 0 4
            -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, // 1 5
            -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, // 5 6
            -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, // 4 7
            // TOP
            -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, // 1 8
            -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, // 5 9
             0.5f,  0.5f,  0.5f, 1.0f, 0.0f, // 6 10
             0.5f, -0.5f,  0.5f, 0.0f, 0.0f, // 2 11
             // RIGHT
             0.5f, -0.5f,  0.5f, 0.0f, 1.0f, // 2 12
             0.5f,  0.5f,  0.5f, 1.0f, 1.0f, // 6 13
             0.5f,  0.5f, -0.5f, 1.0f, 0.0f, // 7 14
             0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // 3 15
             // BOTTOM
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // 0 16
            -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, // 4 17
             0.5f,  0.5f, -0.5f, 1.0f, 0.0f, // 7 18
             0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // 3 19
             //BACK
            -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, // 4 20
            -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, // 5 21
             0.5f,  0.5f,  0.5f, 1.0f, 0.0f, // 6 22
             0.5f,  0.5f, -0.5f, 0.0f, 0.0f, // 7 23
        };

        const unsigned int BoxIdxBuff[] = {
            0, 1, 2,
            2, 3, 0,
            4, 5, 6,
            6, 7, 4,
            8, 9, 10,
            10, 11, 8,
            12, 13, 14,
            14, 15, 12,
            16, 17, 18,
            18, 19, 16,
            20, 21, 22,
            22, 23, 20
        };

    }

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


	class Box
	{
	public:
        Box() : 
            mVertexBuffer{ BoxVertexData::BoxVertexBuff, 5 * 4 * 6 * sizeof(float) },
            mIndexBuffer{ BoxVertexData::BoxIdxBuff, 3 * 12 },
            mTransform(glm::mat4(1.0f))
        {
            VertexBufferLayout layout;
            layout.Push<float>(3);
            layout.Push<float>(2);

            mVertexArray.AddBuffer(mVertexBuffer, layout);

            //mIndexBuffer.Unbind();
            //mVertexArray.Unbind();
            //mVertexBuffer.Unbind();
        }

        Box(glm::vec3 pos) : Box()
        {
            SetPosition(pos);
        }

        Box(glm::vec3 pos, glm::vec3 scale) : Box(pos)
        {
            SetScale(scale);
        }

        Box(glm::vec3 pos, float angle_rad, glm::vec3 axis) : Box(pos)
        {
            SetRotation(angle_rad, axis);
        }

        ~Box()
        {
            /*delete mVertexArray;
            delete mIndexBuffer;
            delete mVertexBuffer;*/
        }

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


    class Billboard {

    public:
        Billboard() :
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
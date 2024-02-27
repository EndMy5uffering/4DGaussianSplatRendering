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

    struct Vertex2D {
        glm::vec2 Position;
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

    const Vertex2D QuadVerteices[] = {
        {{  0.5f,  0.5f }},
        {{  0.5f, -0.5f }},
        {{ -0.5f, -0.5f }},
        {{ -0.5f,  0.5f }},
    };
    const unsigned int QuadIdxBufferData[] = { 0,2,1,2,0,3 };
    
    struct Quad 
    {

        VertexBuffer QuadVB{ QuadVerteices, 4 * sizeof(Vertex2D) };
        IndexBuffer QuadIdxBuffer{ QuadIdxBufferData, 6 };
        VertexArray QuadVA{};
        VertexBufferLayout QuadVBLayout{};

        Quad() 
        {
            this->QuadVBLayout.Push<glm::vec2>();
            this->QuadVA.AddBuffer(this->QuadVB, this->QuadVBLayout);
        }

        ~Quad() { }
    };
}
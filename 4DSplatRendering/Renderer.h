#pragma once

#include <GLEW/glew.h>
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Camera.h"

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

class Renderer
{
public:
    
    Renderer() {
        mLine2D.AddShaderSource("../Shader/Line2DFrag.GLSL", GL_FRAGMENT_SHADER);
        mLine2D.AddShaderSource("../Shader/Line2DVert.GLSL", GL_VERTEX_SHADER);
        mLine2D.BuildShader();
        mLine3D.AddShaderSource("../Shader/LineFrag.GLSL", GL_FRAGMENT_SHADER);
        mLine3D.AddShaderSource("../Shader/LineVert.GLSL", GL_VERTEX_SHADER);
        mLine3D.BuildShader();
    }

    void Clear() const;
    void Draw(const VertexArray& va, const IndexBuffer& ib) const;
    void DrawLine(glm::vec3 v0, glm::vec3 v1, glm::vec4 color, Camera& cam, float thickness);
    void DrawLine(glm::vec3 v0, glm::vec3 v1, glm::vec4 color, Camera& cam);
    void DrawLine(glm::vec2 v0, glm::vec2 v1, glm::vec4 color);

private:
    Shader mLine3D;
    Shader mLine2D;
};


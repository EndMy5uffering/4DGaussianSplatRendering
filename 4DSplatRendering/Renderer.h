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
    
    void Clear() const;
    void Draw(const VertexArray& va, const IndexBuffer& ib) const;
    void DrawLine(glm::vec3 v0, glm::vec3 v1) const;

private:

};


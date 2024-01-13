#include "Renderer.h"

#include <iostream>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error]: ( " << error << " ) " << function << " : " << file << " : " << line << std::endl;
        return false;
    }
    return true;
}

void Renderer::Clear() const
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib) const
{
    va.Bind();
    ib.Bind();

    GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::DrawLine(glm::vec3 v0, glm::vec3 v1) const
{
    float vertBuff[6] = {
        v0.x, v0.y, v0.z,
        v1.x, v1.y, v1.z
    };

    GLuint vertexbuffer;
    GLCall(glGenBuffers(1, &vertexbuffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertBuff), vertBuff, GL_STATIC_DRAW));

    GLuint array;
    glGenVertexArrays(1, &array);
    glBindVertexArray(array);


    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(
        0,                  
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    ));

    GLCall(glDrawArrays(GL_LINES, 0, 2));

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &array);
}


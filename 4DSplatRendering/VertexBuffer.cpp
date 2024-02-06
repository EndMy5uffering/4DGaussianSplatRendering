#include "VertexBuffer.h"

#include "Renderer.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size)
{
    GLCall(glGenBuffers(1, &m_RendererID));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
    if (data) 
    {
        mIsDynamic = false;
        GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
    }
    else 
    {
        mIsDynamic = true;
        GLCall(glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
    }
}

VertexBuffer::~VertexBuffer()
{
    GLCall(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBuffer::SubData(unsigned int offset, const void* data, unsigned int size) const
{
    Bind();
    if (mIsDynamic)
    {
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
    }
}

void VertexBuffer::SubData(const void* data, unsigned int size) const
{
    Bind();
    if (mIsDynamic)
    {
        GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, size, data));
    }
}

bool VertexBuffer::isDynamic()
{
    return mIsDynamic;
}

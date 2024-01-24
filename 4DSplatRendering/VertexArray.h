#pragma once
#include "VertexBuffer.h"

class VertexBufferLayout;

class VertexArray
{
public:
	VertexArray(VertexArray&) = delete;
	VertexArray();
	~VertexArray();

	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
	void AddBuffer(const VertexBuffer& vb, unsigned int layout, unsigned int numComponents, unsigned int type, unsigned int stride, void* offset);

	void Bind() const;
	void Unbind() const;

private:
	unsigned int m_RendererID;
};


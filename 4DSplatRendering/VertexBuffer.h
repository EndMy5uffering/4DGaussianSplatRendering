#pragma once

#include<vector>
#include "glm/glm.hpp"

class VertexBuffer
{
public:
	VertexBuffer(VertexBuffer&) = delete;
	VertexBuffer(const void* data, unsigned int size);

	~VertexBuffer();

	void Bind() const;
	void Unbind() const;

private:
	unsigned int m_RendererID;

};


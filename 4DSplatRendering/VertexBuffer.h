#pragma once

#include<vector>
#include "glm/glm.hpp"

class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int size);

	~VertexBuffer();

	void Bind() const;
	void Unbind() const;
	void SubData(unsigned int offset,const void* data, unsigned int size) const;
	void SubData(const void* data, unsigned int size) const;

	bool isDynamic();

private:
	unsigned int m_RendererID;
	bool mIsDynamic = false;
};

/*
	The basic implementation of this class is from a youtube tutorial series:
	https://www.youtube.com/watch?v=W3gAzLwfIP0&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2
	Some changes were made to make it better fit into the project
*/

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

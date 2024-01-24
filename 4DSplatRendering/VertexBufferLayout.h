#pragma once
#include <vector>
#include <GLEW/glew.h>
#include "Renderer.h"

struct VertexBufferElement 
{
	unsigned int type;
	unsigned int count;
	unsigned int normalized;
	unsigned int customOffset = 0;
	bool useCustomOffset = false;

	static unsigned int GetSizeOfType(unsigned int type) 
	{
		switch (type)
		{
		case GL_FLOAT: 
			return 4;
		case GL_UNSIGNED_INT: 
			return 4;
		case GL_UNSIGNED_BYTE:
			return 1;
		default:
			ASSERT(false);
			return 0;
		}
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_elements;
	unsigned int m_Stride;

public:
	VertexBufferLayout(VertexBufferLayout&) = delete;
	VertexBufferLayout() : m_Stride(0)
	{}

	template<typename T>
	void Push(unsigned int count)
	{
		//static_assert(false);
	}

	template<>
	void Push<float>(unsigned int count) 
	{
		m_elements.push_back({ GL_FLOAT, count, GL_FALSE });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_FLOAT);
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		m_elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT);
	}

	template<>
	void Push<unsigned char>(unsigned int count)
	{
		m_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_Stride += count * VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE);
	}

	template<>
	void Push<glm::mat4>(unsigned int count)
	{
		m_elements.push_back({ GL_FLOAT, 4, GL_FALSE });
		m_Stride += sizeof(glm::mat4);
	}

	inline const std::vector<VertexBufferElement>& GetElements() const { return m_elements; };
	inline unsigned int GetStride() const { return m_Stride; };
};


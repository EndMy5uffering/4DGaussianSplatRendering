#include "VertexArray.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"

VertexArray::VertexArray()
{
	GLCall(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray()
{
	GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
	Bind();
	vb.Bind();
	
	const auto& elemetns = layout.GetElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elemetns.size(); ++i) 
	{
		const auto& element = elemetns[i];
		GLCall(glEnableVertexAttribArray(i));
		GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)offset));
		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}

}

void VertexArray::AddBuffer(const VertexBuffer& vb, unsigned int layout, unsigned int numComponents, unsigned int type, unsigned int stride, void* offset)
{
	Bind();
	vb.Bind();

	GLCall(glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset));
	GLCall(glEnableVertexAttribArray(layout));

}

void VertexArray::Bind() const
{
	GLCall(glBindVertexArray(m_RendererID));
}

void VertexArray::Unbind() const
{
	GLCall(glBindVertexArray(0));
}

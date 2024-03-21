#pragma once

/*
	The basic implementation of this class is from a youtube tutorial series:
	https://www.youtube.com/watch?v=W3gAzLwfIP0&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2
	Some changes were made to make it better fit into the project
*/

class IndexBuffer
{
public:
	IndexBuffer(const unsigned int* data, unsigned int count);

	~IndexBuffer();

	void Bind() const;
	void Unbind() const;

	inline unsigned int GetCount() const { return m_Count; }

	void SubData(unsigned int offset, const void* data, unsigned int size) const;

private:
	unsigned int m_RendererID;
	unsigned int m_Count;
	bool mIsDynamic;
};

#pragma once

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

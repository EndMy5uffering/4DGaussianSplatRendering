/*
	As the basic structure for the different buffers is the same
	this class was copied form the other buffer classes and ajusted to handle SSBOs
*/

#pragma once

#include "Renderer.h"

class ShareStorageBuffer
{
public:
	ShareStorageBuffer(const void* data, unsigned int size);

	~ShareStorageBuffer();

	void Bind() const;
	void Bind(int position) const;
	void Unbind() const;
	void SubData(unsigned int offset, const void* data, unsigned int size) const;
	void SubData(const void* data, unsigned int size) const;

	bool isDynamic() { return this->mIsDynamic; }

private:
	unsigned int m_RendererID;
	bool mIsDynamic = false;

};


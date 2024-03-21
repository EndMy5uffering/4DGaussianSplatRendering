#pragma once

#include <string>
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>

class Texture
{

public:
	
	Texture(int width, int height, GLenum internalFormat, GLenum format, GLenum type);
	Texture(std::string path, GLenum internalFormat, GLenum format);
	~Texture();

	void Bind(int unit);
	void BindAsComputeTexture(int unit);
	void Unbind();
	int GetWidth() { return this->m_Width; }
	int GetHeight() { return this->m_Height; }

private:
	int m_Width;
	int m_Height;

	unsigned int m_TextureID;

};


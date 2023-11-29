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
	int GetWidth() { return this->mWidth; }
	int GetHeight() { return this->mHeight; }

private:
	int mWidth;
	int mHeight;

	unsigned int mTextureID;

};


#include "Texture.h"


Texture::Texture(int width, int height, GLenum internalFormat, GLenum format, GLenum type) : mWidth(width), mHeight(height)
{
    glGenTextures(1, &mTextureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
}

Texture::Texture(std::string path, GLenum internalFormat, GLenum format)
{
    int img_width, img_height, numColCh;
    unsigned char* bytes = stbi_load(path.c_str(), &img_width, &img_height, &numColCh, 0);

    if (bytes) 
    {
        mWidth = img_width;
        mHeight = img_height;

        glGenTextures(1, &mTextureID);
        glBindTexture(GL_TEXTURE_2D, mTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, img_width, img_height, 0, format, GL_UNSIGNED_BYTE, bytes);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(bytes);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else 
    {
        std::cout << "Could not load image: " << path << "\n";
        mWidth = -1;
        mHeight = -1;
        mTextureID = -1;
    }


}

Texture::~Texture()
{
    glDeleteTextures(1, &mTextureID);
}

void Texture::Bind(int unit)
{
    //glBindTexture(GL_TEXTURE_2D, mTextureID);
    glBindTextureUnit(unit, mTextureID);
}

void Texture::BindAsComputeTexture(int unit)
{
    glBindImageTexture(unit, mTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

void Texture::Unbind()
{

}

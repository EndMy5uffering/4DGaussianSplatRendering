#include "Texture.h"


Texture::Texture(int width, int height, GLenum internalFormat, GLenum format, GLenum type) : m_Width(width), m_Height(height)
{
    glGenTextures(1, &m_TextureID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
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
        m_Width = img_width;
        m_Height = img_height;

        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
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
        m_Width = -1;
        m_Height = -1;
        m_TextureID = -1;
    }


}

Texture::~Texture()
{
    glDeleteTextures(1, &m_TextureID);
}

void Texture::Bind(int unit)
{
    //glBindTexture(GL_TEXTURE_2D, mTextureID);
    glBindTextureUnit(unit, m_TextureID);
}

void Texture::BindAsComputeTexture(int unit)
{
    glBindImageTexture(unit, m_TextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

void Texture::Unbind()
{

}

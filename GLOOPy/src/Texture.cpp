#include "Texture.h"

Texture::Texture()
{
    glGenTextures(1, &ID);
}

void Texture::bind(int number)
{
    glActiveTexture(GL_TEXTURE0 + number);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::bind_cubeMap(int number)
{
    glActiveTexture(GL_TEXTURE0 + number);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
}

void Texture::setTextureParameter_XY(GLint parameter)
{
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, parameter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, parameter);
}

void Texture::setTextureParameter_filter(GLint parameter)
{
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, parameter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, parameter);
}

void Texture::setSkyboxParameter_XY(GLint parameter)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, parameter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, parameter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, parameter);
}

void Texture::setSkyboxParameter_filter(GLint parameter)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, parameter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, parameter);
}

bool Texture::loadImage(const char* path)
{
    int width = 0;
    int height = 0;
    int nrChannels = 0;

    unsigned char* data =
        stbi_load(path, &width, &height, &nrChannels, 0);

    // Check whether the image actually loaded BEFORE uploading it.
    if (data == nullptr)
    {
        std::cout << "FAILED to load texture: " << path << '\n';
        std::cout << "Reason: " << stbi_failure_reason() << '\n';
        return false;
    }

    GLenum format = GL_RGB;



    if (nrChannels == 1) format = GL_RED;
    else if (nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, ID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(GL_TEXTURE_2D);

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    stbi_image_free(data);
    return true;
}

void Texture::loadSkybox(const char* path, int number)
{
    int width = 0;
    int height = 0;
    int nrChannels = 0;

    unsigned char* data =
        stbi_load(path, &width, &height, &nrChannels, 0);

    // Check whether the image actually loaded BEFORE uploading it.
    if (data == nullptr)
    {
        std::cout << "FAILED to load texture: " << path << '\n';
        std::cout << "Reason: " << stbi_failure_reason() << '\n';
    }

    GLenum format = GL_RGB;

    if (nrChannels == 1) format = GL_RED;
    else if (nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;

    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + number, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

}

void Texture::free()
{
    glDeleteTextures(1, &ID);
}


#pragma once

#include <glad/glad.h>
#include <iostream>
#include <stb_image.h>

class Texture
{
public:
    unsigned int ID;
    std::string type;
    std::string path;

    Texture();
    void bind(int number);
    void bind_cubeMap(int number);
    void setTextureParameter_XY(GLint parameter);
    void setTextureParameter_filter(GLint parameter);
    void setSkyboxParameter_XY(GLint parameter);
    void setSkyboxParameter_filter(GLint parameter);
    bool loadImage(const char* path);
    void loadSkybox(const char* path, int number);
    void free();
    
};
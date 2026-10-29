#pragma once

#include <glad/glad.h>
#include <iostream>

class Buffer
{
public:
    unsigned int ID = 0;
    GLenum kind;

    Buffer();

    void upload(GLenum target, const void* data, std::size_t byteSize);

    void free();

};
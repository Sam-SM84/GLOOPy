#pragma once
#include <glad/glad.h>
class VertexArray
{
public:
    unsigned int ID;

    VertexArray();


    void bind();


    void unbind();


    void free();

};
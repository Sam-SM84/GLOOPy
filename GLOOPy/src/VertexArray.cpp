#include "VertexArray.h"

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &ID);
}

void VertexArray::bind()
{
    glBindVertexArray(ID);
}

void VertexArray::unbind()
{
    glBindVertexArray(0);
}

void VertexArray::free()
{
    glDeleteVertexArrays(1, &ID);
}
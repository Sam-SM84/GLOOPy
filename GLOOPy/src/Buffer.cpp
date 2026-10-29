#include "Buffer.h"

Buffer::Buffer()
{
    glGenBuffers(1, &ID);
}

void Buffer::upload(GLenum target, const void* data, std::size_t byteSize)
{
    kind = target;
    glBindBuffer(kind, ID);
    glBufferData(kind, byteSize, data, GL_STATIC_DRAW);
}

void Buffer::free()
{
    glDeleteBuffers(1, &ID);
}
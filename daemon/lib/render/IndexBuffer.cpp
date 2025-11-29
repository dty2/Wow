#include "render/IndexBuffer.h"

#include <GLES3/gl32.h>

#include "render/error.h"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count)
    : count(count) {
  GLCall(glGenBuffers(1, &id));
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id));
  GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int),
                      data, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer() { GLCall(glDeleteBuffers(1, &id)); }

void IndexBuffer::bind() const {
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id));
}

void IndexBuffer::unBind() const {
  GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

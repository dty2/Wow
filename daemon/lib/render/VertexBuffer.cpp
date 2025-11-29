#include "render/VertexBuffer.h"

#include <GLES3/gl32.h>

#include "render/error.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size) {
  GLCall(glGenBuffers(1, &id));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, id));
  GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer() { GLCall(glDeleteBuffers(1, &id)); }

void VertexBuffer::bind() const { GLCall(glBindBuffer(GL_ARRAY_BUFFER, id)); }

void VertexBuffer::unBind() const { GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0)); }

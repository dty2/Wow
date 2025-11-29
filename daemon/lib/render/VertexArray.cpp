#include "render/VertexArray.h"

#include "render/error.h"

VertexArray::VertexArray() { GLCall(glGenVertexArrays(1, &id)); }

VertexArray::~VertexArray() { GLCall(glDeleteVertexArrays(1, &id)); }

void VertexArray::bind() const { GLCall(glBindVertexArray(id)); }

void VertexArray::unBind() const { GLCall(glBindVertexArray(0)); }

void VertexArray::addBuffer(const VertexBuffer& vb,
                            const VertexBufferLayout& layout,
                            unsigned int baseIndex) {
  bind();
  vb.bind();
  const auto& elements = layout.getElements();
  unsigned int offset = 0;
  for (unsigned int i = 0; i < elements.size(); i++) {
    const auto& element = elements[i];
    auto index = i + baseIndex;
    GLCall(glEnableVertexAttribArray(index));
    GLCall(glVertexAttribPointer(index, element.count, element.type,
                                 element.normalized, layout.getStride(),
                                 reinterpret_cast<const void*>(offset)));
    offset += element.count * VertexBufferElement::getSizeOfType(element.type);
  }
}

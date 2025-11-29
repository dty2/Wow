#include "render/VertexBufferLayout.h"

#include <GLES3/gl32.h>

template <>
void VertexBufferLayout::push<float>(unsigned int count) {
  elements.push_back({count, GL_FLOAT, false});
  stride += count * VertexBufferElement::getSizeOfType(GL_FLOAT);
}

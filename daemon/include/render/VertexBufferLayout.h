#pragma once

#include <GLES3/gl32.h>

#include <cassert>
#include <vector>

#include "error.h"

struct VertexBufferElement {
  unsigned int count;
  unsigned int type;
  unsigned char normalized;

  static unsigned int getSizeOfType(unsigned int type) {
    switch (type) {
      case GL_FLOAT:
        return 4;
      case GL_UNSIGNED_INT:
        return 4;
      case GL_UNSIGNED_BYTE:
        return 1;
    }
    ASSERT(false);
    return 0;
  }
};

class VertexBufferLayout {
 private:
  std::vector<VertexBufferElement> elements;
  unsigned int stride;

 public:
  VertexBufferLayout() : stride(0) {};

  template <typename T>
  void push(unsigned int count) {
    assert(false);
  }

  inline const std::vector<VertexBufferElement> getElements() const {
    return elements;
  }

  inline unsigned int getStride() const { return stride; }
};

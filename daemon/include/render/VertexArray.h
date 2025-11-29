#pragma once

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class VertexArray {
 private:
  unsigned int id;

 public:
  VertexArray();
  ~VertexArray();

  void bind() const;
  void unBind() const;
  void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout,
                 unsigned int baseIndex = 0);
};

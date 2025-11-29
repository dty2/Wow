#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "render/Shader.h"

class FrameBuffer;

class Renderer {
 private:
  std::atomic_bool& signal;
  static std::string vertexShader;
  static std::string fragmentShader;

  std::unique_ptr<Shader> shader;
  std::unique_ptr<VertexArray> va;
  std::unique_ptr<VertexBuffer> vb;
  std::unique_ptr<VertexBuffer> tb;
  std::unique_ptr<IndexBuffer> ib;

  int width;
  int height;

  bool isDynamic = false;
  unsigned int frameDelay;

  FrameBuffer& frameBuffer;

 public:
  void prepareRender(int width, int height, bool isDynamic,
                     unsigned int frameDelay = 0);
  void render(bool& decodeFinished);
  bool initRenderer();
  Renderer(std::atomic_bool& signal, FrameBuffer& buffer);
  ~Renderer();
};

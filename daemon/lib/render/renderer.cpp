#include "render/renderer.h"

#include <GLES3/gl32.h>

#include <memory>
#include <thread>

#include "player.h"
#include "render/Shader.h"
#include "render/Texture.h"
#include "window.h"

std::string Renderer::vertexShader = R"(
    #version 320 es

    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec2 aTex;
    out vec2 vTex;

    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        vTex = aTex;
    }

)";

std::string Renderer::fragmentShader = R"(
    #version 320 es
    precision mediump float;

    in vec2 vTex;
    out vec4 fragColor;

    uniform sampler2D uTexture;

    void main() {
        fragColor = texture(uTexture, vTex);
    }
)";

bool Renderer::initRenderer() {
  // create opengl context
  createWindow();

  GLCall(glViewport(0, 0, 1920, 1080));

  shader = std::make_unique<Shader>(vertexShader, fragmentShader);
  shader->bind();
  shader->setUniform1i("uTexture", 0);

  // clang-format off
  const GLfloat quadVertices[] = {
    -1.0f,  -1.0f,  // left down
    1.0f,   -1.0f,  // right down
    -1.0f,  1.0f,   // left up
    1.0f,   1.0f    // right up
  };

  unsigned int indices[] = {
    0, 1, 2,
    2, 3, 1
  };
  // clang-format on

  va = std::make_unique<VertexArray>();
  vb = std::make_unique<VertexBuffer>(quadVertices, sizeof(quadVertices));

  VertexBufferLayout layout;
  layout.push<float>(2);
  va->addBuffer(*vb, layout, 0);
  va->bind();

  ib = std::make_unique<IndexBuffer>(indices, 6);
  ib->bind();

  return true;
}

void Renderer::prepareRender(int width, int height, bool isDynamic,
                             unsigned int frameDelay) {
  this->width = width;
  this->height = height;
  this->isDynamic = isDynamic;
  this->frameDelay = frameDelay;

  float scale =
      std::max((float)window.width / width, (float)window.height / height);

  int scaledWidth = static_cast<int>(width * scale);
  int scaledHeight = static_cast<int>(height * scale);

  float texLeft = 0.5f - ((float)window.width / (2.0f * scaledWidth));
  float texRight = 0.5f + ((float)window.width / (2.0f * scaledWidth));
  float texBottom = 0.5f - ((float)window.height / (2.0f * scaledHeight));
  float texTop = 0.5f + ((float)window.height / (2.0f * scaledHeight));

  // clang-format off
  const GLfloat texturesVertices[] = {
    texLeft,    texTop,
    texRight,   texTop,
    texLeft,    texBottom,
    texRight,   texBottom
  };

  // clang-format on
  tb = std::make_unique<VertexBuffer>(texturesVertices,
                                      sizeof(texturesVertices));

  VertexBufferLayout layout;
  layout.push<float>(2);
  va->addBuffer(*tb, layout, 1);
  va->bind();
}

void Renderer::render(bool& decodeFinished) {
  Texture texture(width, height);

  auto renderOnce = [&] {
    oneFrame buffer = frameBuffer.get();

    texture.bind();
    texture.update(buffer);
    GLCall(glDrawElements(GL_TRIANGLES, (*ib).getCount(), GL_UNSIGNED_INT,
                          nullptr));
    changeSurface();
  };

  if (!isDynamic) {
    renderOnce();
    return;
  }

  while (true) {
    renderOnce();
    std::this_thread::sleep_for(std::chrono::milliseconds(this->frameDelay));

    if (decodeFinished) {
      if (frameBuffer.empty()) {
        break;
      }
    }

    if (signal) {
      // To prevent renderer quit first but decoder still wait in push
      frameBuffer.notify();
      break;
    }
  }
}

Renderer::Renderer(std::atomic_bool& signal, FrameBuffer& frameBuffer)
    : signal(signal), frameBuffer(frameBuffer) {}

Renderer::~Renderer() { releaseWindow(); }

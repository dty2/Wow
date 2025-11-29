#include "render/Texture.h"

#include <GLES3/gl32.h>

#include "render/error.h"

Texture::Texture(int width, int height) : width(width), height(height) {
  GLCall(glGenTextures(1, &id));
  GLCall(glBindTexture(GL_TEXTURE_2D, id));
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  GLCall(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
  GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                      GL_UNSIGNED_BYTE, nullptr));
}

Texture::~Texture() { GLCall(glDeleteTextures(1, &id)); }

void Texture::update(std::vector<uint8_t>& localBuffer) {
  GLCall(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB,
                         GL_UNSIGNED_BYTE, localBuffer.data()));
}

void Texture::bind(unsigned int slot) {
  GLCall(glActiveTexture(GL_TEXTURE0 + slot));
  GLCall(glBindTexture(GL_TEXTURE_2D, id));
}

void Texture::unBind() { GLCall(glBindTexture(GL_TEXTURE_2D, 0)); }

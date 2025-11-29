#include <GLES3/gl32.h>

#include "log.hpp"

void glCheckError(const char* function) {
  while (GLenum error = glGetError()) {
    LOG(ERROR) << "OpenGL error in " << function << ": " << error;
  }
}

void glClearError() { while (glGetError() != GL_NO_ERROR); }

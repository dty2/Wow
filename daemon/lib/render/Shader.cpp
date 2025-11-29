#include "render/Shader.h"

#include <GLES3/gl32.h>

#include "log.hpp"
#include "render/error.h"

unsigned int Shader::compileShader(unsigned int type,
                                   const std::string& source) {
  unsigned int id = glCreateShader(type);
  const char* src = source.c_str();
  GLCall(glShaderSource(id, 1, &src, nullptr));
  GLCall(glCompileShader(id));

  int result;
  GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
  if (result == GL_FALSE) {
    int length;
    GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
    std::string message(length, '\0');
    GLCall(glGetShaderInfoLog(id, length, &length, message.data()));
    auto typeInfo = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
    LOG(ERROR) << "Failed to compile " << typeInfo;
    LOG(ERROR) << message;
    GLCall(glDeleteShader(id));
    return 0;
  }

  return id;
}

int Shader::createShader() {
  unsigned int program = glCreateProgram();
  unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
  unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

  if (!vs || !fs) {
    return -1;
  }

  GLCall(glAttachShader(program, vs));
  GLCall(glAttachShader(program, fs));
  GLCall(glLinkProgram(program));
  GLCall(glValidateProgram(program));
  GLCall(glDeleteShader(vs));
  GLCall(glDeleteShader(fs));

  return program;
}

Shader::Shader(std::string vertexShader, std::string fragmentShader)
    : vertexShader(vertexShader), fragmentShader(fragmentShader) {
  id = createShader();

  if (id == -1) {
    LOG(ERROR) << "Create Shader failed";
    return;
  }
}

Shader::~Shader() { GLCall(glDeleteProgram(id)); }

void Shader::bind() { GLCall(glUseProgram(id)); }

void Shader::unBind() { GLCall(glUseProgram(0)); }

void Shader::setUniform1i(const std::string& name, int v0) {
  GLCall(glUniform1i(getUniformLocation(name), v0));
}

int Shader::getUniformLocation(const std::string& name) {
  GLCall(int loc = glGetUniformLocation(id, name.c_str()));
  if (loc == -1)
    LOG(WARNING) << "Uniform " << name << " not found in shader program!";
  return loc;
}

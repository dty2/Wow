#pragma once

#include <string>

class Shader {
 private:
  unsigned int id;
  unsigned int compileShader(unsigned int type, const std::string& source);
  int createShader();

  std::string vertexShader;
  std::string fragmentShader;

 public:
  Shader(std::string vertexShader, std::string fragmentShader);
  ~Shader();

  void bind();
  void unBind();
  void setUniform1i(const std::string& name, int v0);
  int getUniformLocation(const std::string& name);
};

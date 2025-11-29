#pragma once

#include <cstdint>
#include <vector>

class Texture {
 private:
  unsigned int id;
  int width;
  int height;

 public:
  Texture(int width, int height);
  ~Texture();

  void bind(unsigned int slot = 0);
  void unBind();
  void update(std::vector<uint8_t>& localBuffer);
  int getWidth() const { return width; };
  int getHeight() const { return height; };
};

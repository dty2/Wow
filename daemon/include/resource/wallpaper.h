#pragma once

#include <cstring>
#include <string>
#include <vector>

#include "log.hpp"

enum WallPaperType { NONE, STATIC, DYNAMIC };
using wpType = WallPaperType;

struct WallPaper {
  std::string name;
  std::string path;
  WallPaperType type;
  struct {
    int width;
    int height;
    int frameDelay;
  } info;
  static wpType isWallPaper(std::string filename);
};

class WallPaperList {
 public:
  std::string name;
  std::string dir;
  int time;
  std::vector<WallPaper> list;

  WallPaperList() = default;
  WallPaperList(std::string name);

  void add(WallPaper wallpaper);
  int size() const;

  WallPaper& operator[](size_t index) {
    if (index >= list.size()) {
      LOG(ERROR) << "Index out of range";
    }
    return list[index];
  }

  const WallPaper& operator[](size_t index) const {
    if (index >= list.size()) {
      LOG(ERROR) << "Index out of range";
    }
    return list[index];
  }
};

using wpList = class WallPaperList;

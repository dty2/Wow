#ifndef PAPER_H
#define PAPER_H

#include "Common.h"

namespace Wow {

class WallPaper;
class StaticWallPaper;
class DynamicWallPaper;
template <typename T>
class WallPaperList;

using WallPaperPtr = std::shared_ptr<WallPaper>;
using StaticWallPaperPtr = std::shared_ptr<StaticWallPaper>;
using DynamicWallPaperPtr = std::shared_ptr<DynamicWallPaper>;

using StaticWallPaperList = WallPaperList<StaticWallPaper>;
using DynamicWallPaperList = WallPaperList<DynamicWallPaper>;

class WallPaper {
 public:
  std::string name, path;
  enum Type { STATIC, DYNAMIC } type;
  struct Size {
    int width;
    int height;
  } size;
};

class StaticWallPaper : public WallPaper {
  static inline std::unordered_map<std::string, bool> extension = {
      {".webp", 1}, {".jpg", 1},  {".jpeg", 1}, {".svg", 1},
      {".bmp", 1},  {".heif", 1}, {".tiff", 1}, {".png", 1}};

 public:
  std::vector<uint8_t> buffer;
  static bool isExtension(std::string e);
};

class DynamicWallPaper : public WallPaper {
  static inline std::unordered_map<std::string, bool> extension = {
      {".webm", 1}, {".mp4", 1}, {".mkv", 1}, {".mov", 1}};

 public:
  std::mutex mtx;
  std::queue<std::vector<uint8_t>> buffer;
  int frameDelay;
  static bool isExtension(std::string e);
};

template <typename T>
class WallPaperList {
  std::string listName;
  WallPaper::Type type;
  std::vector<T> list;

 public:
  WallPaperList() = default;
  WallPaperList(std::string listName) : listName(listName) {}
  int getSize() { return list.size(); }
  std::string getName() { return listName; }
  void setType(WallPaper::Type type) { this->type = type; }
  void push(WallPaperPtr wallpaper) { list.push_back(wallpaper); }
  WallPaper::Type getType() { return type; }
  T& operator[](size_t index) {
    if (index >= list.size()) {
      throw std::out_of_range("Index out of range");
    }
    return list[index];
  }
};

}  // namespace Wow

#endif  // PAPER_H

#ifndef PAPER_H
#define PAPER_H

#include <memory>

#include "common.h"

namespace Wow {

class Wallpaper {
 public:
  struct Size {
    int scaledWidth;
    int scaledHeight;
  } size;
  std::string name;
  enum Type { STATIC, DYNAMIC } type;
};

class StaticWallpaper : public Wallpaper {
 public:
  std::vector<uint8_t> buffer;
};

class DynamicWallpaper : public Wallpaper {
 public:
  std::vector<std::vector<uint8_t>> bufferStream;
  int frameDelay;
};

class WallpaperManager {
  std::string workDir = getenv("HOME") + std::string("/.config/Wow");
  const std::string staticDir = workDir + "/static";
  const std::string dynamicDir = workDir + "/dynamic";

  void loadPaper(std::filesystem::directory_entry entry);

 public:
  WallpaperManager();

  std::vector<std::shared_ptr<Wallpaper>> staticWallpapers;
  std::vector<std::shared_ptr<Wallpaper>> dynamicWallpapers;
  std::pair<int, int> loadPapers();
};

}  // namespace Wow

#endif  // PAPER_H

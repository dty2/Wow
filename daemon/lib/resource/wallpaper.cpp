#include "resource/wallpaper.h"

#include <algorithm>
#include <filesystem>
#include <vector>

wpType WallPaper::isWallPaper(std::string filename) {
  static std::vector<std::string> picture_extension = {
      ".webp", ".jpg", ".jpeg", ".svg", ".bmp", ".heif", ".tiff", ".png"};
  static std::vector<std::string> video_extension = {".webm", ".mp4", ".mkv",
                                                     ".mov"};

  const auto& entry = std::filesystem::directory_entry(filename);
  std::string extension = entry.path().extension().string();

  if (std::find(picture_extension.begin(), picture_extension.end(),
                extension) != picture_extension.end()) {
    return STATIC;
  }

  if (std::find(video_extension.begin(), video_extension.end(), extension) !=
      video_extension.end()) {
    return DYNAMIC;
  }

  return NONE;
}

void WallPaperList::add(WallPaper wallpaper) {
  list.push_back(std::move(wallpaper));
}

int WallPaperList::size() const { return list.size(); }

WallPaperList::WallPaperList(std::string name) : name(name) {}

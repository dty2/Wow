#include "paper.h"

namespace Wow {

std::pair<int, int> WallpaperManager::loadPapers() {
  for (const auto& entry : std::filesystem::directory_iterator(staticDir)) {
    loadPaper(entry);
  }
  for (const auto& entry : std::filesystem::directory_iterator(dynamicDir)) {
    loadPaper(entry);
  }
  return std::pair<int, int>(staticWallpapers.size(), dynamicWallpapers.size());
}

void WallpaperManager::loadPaper(std::filesystem::directory_entry entry) {
  if (entry.is_regular_file() && entry.path().extension() == ".webp") {
    std::shared_ptr<StaticWallpaper> wallpaper =
        std::make_shared<StaticWallpaper>();
    wallpaper->name = entry.path().stem().string();
    wallpaper->path = entry.path().string();
    wallpaper->type = Wallpaper::STATIC;
    staticWallpapers.push_back(wallpaper);
    LOG(INFO) << "Load paper: " << wallpaper->name
              << "type: " << wallpaper->type << " successful.";
  } else if (entry.is_regular_file() && entry.path().extension() == ".webm") {
    std::shared_ptr<DynamicWallpaper> wallpaper =
        std::make_shared<DynamicWallpaper>();
    wallpaper->name = entry.path().stem().string();
    wallpaper->path = entry.path().string();
    wallpaper->type = Wallpaper::DYNAMIC;
    dynamicWallpapers.push_back(wallpaper);
    LOG(INFO) << "Load paper: " << wallpaper->name
              << "type: " << wallpaper->type << " successful.";
  } else {
    LOG(WARNING) << "Load paper: " << entry.path() << " error.";
    return;
  }
}

WallpaperManager::WallpaperManager() {
  if (!std::filesystem::exists(staticDir)) {
    std::filesystem::create_directory(staticDir);
  }
  if (!std::filesystem::exists(dynamicDir)) {
    std::filesystem::create_directory(dynamicDir);
  }
}

}  // namespace Wow

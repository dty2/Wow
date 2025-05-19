#include "Manager.h"

#include <nlohmann/json.hpp>

#include "WallPaper.h"

extern std::string VERSION;

namespace Wow {

using Json = nlohmann::json;

void Manager::createConfigFile() {
  Json config;
  config["Version"] = VERSION;
  config["StartMode"] = "Static";

  // Set static wallpaper configuration
  config["Static"]["AutoChange"] = true;
  config["Static"]["IntervalTime"] = 60;
  config["Static"]["StartList"] = "Default";
  config["Static"]["Listname"] = Json::array({"Default"});
  config["Static"]["Default"] = Json::array({});
  // Set dynamic wallpaper configuration
  config["Dynamic"]["AutoChange"] = false;
  config["Dynamic"]["Frequency"] = 20;
  config["Dynamic"]["StartList"] = "Default";
  config["Dynamic"]["Listname"] = Json::array({"Default"});
  config["Dynamic"]["Default"] = Json::array({});

  std::ofstream file(configFile);
  if (file.is_open()) {
    file << config.dump(4);
    file.close();
    std::cout << "Configuration File create successful: " << configFile
              << std::endl;
  } else {
    std::cerr << "Can't create Configuration File at " << configFile
              << std::endl;
  }
}

void Manager::init() {
  if (!std::filesystem::exists(workDir)) {
    std::filesystem::create_directory(workDir);
  }

  if (!std::filesystem::exists(staticDir)) {
    std::filesystem::create_directory(staticDir);
  }

  if (!std::filesystem::exists(dynamicDir)) {
    std::filesystem::create_directory(dynamicDir);
  }

  if (!std::filesystem::exists(configFile)) {
    createConfigFile();
  }
}

bool Manager::loadConfigFile() {
  std::ifstream file(configFile);
  Json config = Json::parse(file);
  if (!(config["Version"] == VERSION)) {
    LOG(WARNING) << "Version mismatch";
  }
  if (config["StartMode"].get<std::string>() == "Static") {
    startMode = WallPaper::STATIC;
  } else {
    startMode = WallPaper::DYNAMIC;
  }

  // Static
  staticConfiguration.autoChange = config["Static"]["AutoChange"].get<bool>();
  staticConfiguration.intervalTime =
      config["Static"]["IntervalTime"].get<int>();
  staticConfiguration.startList =
      config["Static"]["StartList"].get<std::string>();
  staticConfiguration.lists =
      config["Static"]["Listname"].get<std::vector<std::string>>();

  for (auto list : staticConfiguration.lists) {
    WallPaperList<WallPaperPtr> wallpaperlist(list);
    wallpaperlist.setType(WallPaper::STATIC);
    for (auto wallpaper :
         config["Static"][list].get<std::vector<std::string>>()) {
      wallpaperlist.push(
          loadWallPaper(staticDir + "/" + wallpaper, WallPaper::STATIC));
    }
    staticLists.emplace(list, wallpaperlist);
  }

  // Dynamic
  dynamicConfiguration.autoChange = config["Dynamic"]["AutoChange"].get<bool>();
  dynamicConfiguration.intervalTime = config["Dynamic"]["Frequency"].get<int>();
  dynamicConfiguration.startList =
      config["Dynamic"]["StartList"].get<std::string>();
  dynamicConfiguration.lists =
      config["Dynamic"]["Listname"].get<std::vector<std::string>>();

  for (auto list : dynamicConfiguration.lists) {
    WallPaperList<WallPaperPtr> wallpaperlist(list);
    wallpaperlist.setType(WallPaper::DYNAMIC);
    for (auto wallpaper :
         config["Dynamic"][list].get<std::vector<std::string>>()) {
      wallpaperlist.push(
          loadWallPaper(dynamicDir + "/" + wallpaper, WallPaper::DYNAMIC));
    }
    dynamicLists.emplace(list, wallpaperlist);
  }

  return true;
}

WallPaperPtr Manager::loadWallPaper(std::string wallPaperPath,
                                    WallPaper::Type type) {
  const auto& entry = std::filesystem::directory_entry(wallPaperPath);
  if (type == WallPaper::STATIC) {
    if (entry.is_regular_file() &&
        StaticWallPaper::isExtension(entry.path().extension())) {
      StaticWallPaperPtr wallpaper = std::make_shared<StaticWallPaper>();
      wallpaper->name = entry.path().stem().string();
      wallpaper->path = entry.path().string();
      wallpaper->type = WallPaper::STATIC;
      LOG(INFO) << "Load paper: " << wallpaper->name
                << "type: " << wallpaper->type << " successful.";
      return static_cast<WallPaperPtr>(wallpaper);
    }
  } else {
    if (entry.is_regular_file() &&
        DynamicWallPaper::isExtension(entry.path().extension())) {
      DynamicWallPaperPtr wallpaper = std::make_shared<DynamicWallPaper>();
      wallpaper->name = entry.path().stem().string();
      wallpaper->path = entry.path().string();
      wallpaper->type = WallPaper::DYNAMIC;
      LOG(INFO) << "Load paper: " << wallpaper->name
                << "type: " << wallpaper->type << " successful.";
      return static_cast<WallPaperPtr>(wallpaper);
    }
  }
  return nullptr;
}

void Manager::manage() {
  if (std::filesystem::exists(configFile)) {
    loadConfigFile();
  } else {
    std::cout << "Configuration file and directory not exist, so create it now."
              << std::endl;
  }
}

WallPaper::Type Manager::getStartMode() { return startMode; }

Manager::Configuration Manager::getConfiguration(WallPaper::Type type) {
  return type == WallPaper::STATIC ? staticConfiguration : dynamicConfiguration;
}

WallPaperList<WallPaperPtr> Manager::getList(std::string listName,
                                             WallPaper::Type type) {
  if (type == WallPaper::STATIC) {
    return staticLists[listName];
  } else {
    return dynamicLists[listName];
  }
}

std::unordered_map<std::string, WallPaperList<WallPaperPtr>> Manager::getLists(
    WallPaper::Type type) {
  if (type == WallPaper::STATIC) {
    return staticLists;
  } else {
    return dynamicLists;
  }
}

}  // namespace Wow

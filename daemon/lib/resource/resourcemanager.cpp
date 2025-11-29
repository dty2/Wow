#include "resource/resourcemanager.h"

#include <filesystem>
#include <string>
#include <toml++/toml.hpp>
#include <vector>

#include "common.h"
#include "log.hpp"
#include "resource/wallpaper.h"

ResourceManager::ResourceManager(std::string configDir) : configDir(configDir) {
  configFilePath = configDir + "/config.toml";
}

WallPaper ResourceManager::loadWallPaper(std::string path) {
  WallPaper wallpaper;

  auto entry = std::filesystem::directory_entry(path);

  switch (WallPaper::isWallPaper(path)) {
    case NONE:
      LOG(INFO) << path + " can't be a wallpaper.";
      return {};
      break;
    case STATIC:
      wallpaper.type = STATIC;
      break;
    case DYNAMIC:
      wallpaper.type = DYNAMIC;
      break;
  }

  wallpaper.name = entry.path().stem().string();
  wallpaper.path = path;

  LOG(INFO) << "Load paper: " << wallpaper.name << " type: " << wallpaper.type
            << " successful.";
  return wallpaper;
}

void ResourceManager::loadWallPaperList(PlayListConfig config) {
  std::filesystem::path listpath(config.dir);

  if (listpath.is_relative()) {
    listpath = std::filesystem::path(configDir) / listpath;
  }

  if (exists(listpath)) {
    std::string listname = listpath.filename().string();
    LOG(INFO) << "Load a wallpaper list: " << listname;

    wplists[listname].name = config.name;
    wplists[listname].dir = config.dir;
    wplists[listname].time = config.time;

    if (config.order.empty()) {
      for (const auto& filename :
           std::filesystem::directory_iterator(listpath)) {
        wplists[listname].add(loadWallPaper(filename.path().string()));
      }

    } else {
      for (auto it : config.order)
        wplists[listname].add(loadWallPaper(listpath.string() + "/" + it));
    }

  } else {
    LOG(ERROR) << "Can't find " << config.dir;
  }
}

bool ResourceManager::loadConfig() {
  config = {};

  if (!exists(configFilePath)) {
    LOG(ERROR) << "Can't find Configuration File." << configFilePath;
    return false;
  }

  auto result = toml::parse_file(configFilePath);
  if (!result) {
    LOG(ERROR) << "TOML parse error: " << result.error().description() << " at "
               << result.error().source().begin;
    return false;
  }

  const auto& data = result.table();

  if (auto startlist = data["startlist"].value<std::string>()) {
    config.startList = *startlist;
  }

  for (auto&& [listname, node] : data) {
    if (listname == "startlist") {
      continue;
    }

    if (!node.is_table()) {
      continue;
    }

    PlayListConfig listconfig;
    listconfig.name = listname;

    if (config.startList.empty()) {
      config.startList = listconfig.name;
    }

    auto tbl = *node.as_table();
    // dir
    if (auto dir = tbl["dir"].value<std::string>()) {
      listconfig.dir = *dir;
    } else {
      listconfig.dir = configDir + "/" + listconfig.name;
    }

    // time
    if (auto time = tbl["time"].value<int>()) {
      listconfig.time = *time;
    } else {
      listconfig.time = -1;
    }

    if (auto arr = tbl["order"].as_array()) {
      for (auto&& elem : *arr) {
        if (auto s = elem.value<std::string>()) {
          listconfig.order.push_back(*s);
        }
      }
    }

    config.listconfigs.push_back(std::move(listconfig));
  }

  return true;
}

bool ResourceManager::loadResource() {
  if (!exists(configDir)) {
    if (!create_directory(configDir)) {
      return false;
    }
  }

  if (!loadConfig()) {
    return false;
  }

  wplists = {};

  for (auto list : config.listconfigs) {
    loadWallPaperList(list);
  }
  return true;
}

std::string ResourceManager::getStartList() { return config.startList; }

std::unordered_map<std::string, wpList>& ResourceManager::getWallPaperLists() {
  return wplists;
}

std::string ResourceManager::getResourceInfo() {
  std::string info = "";
  info += "config file path: ";
  info += configFilePath + "\n";

  info += "all play list:\n";
  for (auto& [key, value] : wplists) {
    info += "play list name: ";
    info += key + "\n";

    info += "play list dir: ";
    info += value.dir + ":\n";

    info += "play method: ";
    if (value.time == -1)
      info += "manual\n";
    else
      info += "auto\n";

    info += "play list size: ";
    info += std::to_string(value.size()) + "\n";
    info += "\n";
  }

  return info;
}

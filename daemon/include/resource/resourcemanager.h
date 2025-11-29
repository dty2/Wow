#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "wallpaper.h"

struct PlayListConfig {
  std::string name;
  std::string dir;
  int time;
  std::vector<std::string> order;
};

struct Config {
  std::string startList;
  std::vector<PlayListConfig> listconfigs;
};

class ResourceManager {
 private:
  std::string configDir;
  std::string configFilePath;

  Config config;
  std::unordered_map<std::string, wpList> wplists;

  WallPaper loadWallPaper(std::string path);
  void loadWallPaperList(PlayListConfig config);
  void loadConfigJson();
  bool loadConfig();

 public:
  ResourceManager(std::string configDir);
  bool loadResource();
  std::string getStartList();
  std::unordered_map<std::string, wpList>& getWallPaperLists();
  std::string getResourceInfo();
};

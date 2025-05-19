#ifndef MANAGER_H
#define MANAGER_H

#include "Common.h"
#include "WallPaper.h"

namespace Wow {

using ListName = std::vector<std::string>;

class Manager {
 private:
  std::string homeDir = getenv("HOME"), workDir = homeDir + "/.config/Wow",
              configFile = workDir + "/Wow.json",
              staticDir = workDir + "/static",
              dynamicDir = workDir + "/dynamic";

  WallPaper::Type startMode;
  struct Configuration {
    bool autoChange;
    int intervalTime;
    std::string startList;
    ListName lists;
  } staticConfiguration, dynamicConfiguration;

  std::unordered_map<std::string, WallPaperList<WallPaperPtr>> staticLists;
  std::unordered_map<std::string, WallPaperList<WallPaperPtr>> dynamicLists;

  // Used by init
  void createConfigFile();

  // Used by manage
  WallPaperPtr loadWallPaper(std::string wallPaperPath, WallPaper::Type type);
  bool loadConfigFile();

 public:
  // Initlization user directory
  void init();
  // Initlization engine context
  void manage();
  // Used by engine context
  WallPaper::Type getStartMode();
  Configuration getConfiguration(WallPaper::Type type);
  WallPaperList<WallPaperPtr> getList(std::string listName,
                                      WallPaper::Type type);
  std::unordered_map<std::string, WallPaperList<WallPaperPtr>> getLists(
      WallPaper::Type type);
};

}  // namespace Wow

#endif  // MANAGER_H

#ifndef ENGINE_H
#define ENGINE_H

#include "Common.h"
#include "Communication.h"
#include "Decoder.h"
#include "Manager.h"
#include "Renderer.h"
#include "WallPaper.h"

namespace Wow {

enum Status { PLAY, STOP };

class EngineContext {
 private:
  Manager manager;
  Status status;
  WallPaper::Type mode;
  struct PlayInfo {
    std::string listIndex;
    int intervalTime;
    bool autoChange;
    int index;
  } info;

  WallPaperList<WallPaperPtr> playlist;

 public:
  bool signal;
  EngineContext();
  std::pair<bool, bool> getAutoChange();
  Status getStatus();
  WallPaper::Type getMode();
  std::string getPlayList();
  int& getIntervalTime();
  WallPaperPtr currentWallPaper();

  // Function
  void toggleStop();
  void next();
  void previous();
  void setPlayList(std::string playListName, WallPaper::Type type);
};

class Engine {
 private:
  EngineContext context;
  Communication communication;
  Renderer renderer;
  Decoder decoder;

  std::function<void(WallPaperPtr wallPaperPtr)> play, playStatic, playDynamic;
  // handle Command
  void handleCommand();
  std::queue<std::string> cmdList;

 public:
  Engine();
  void start();
};

}  // namespace Wow

#endif  // ENGINE_H

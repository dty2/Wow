#ifndef ENGINE_H
#define ENGINE_H

#include "common.h"
#include "communication.h"
#include "paper.h"
#include "renderer.h"

namespace Wow {

// Engine Running Context
class EngineContext {
 private:
  WallpaperManager &manager;
  std::shared_ptr<std::vector<std::shared_ptr<Wallpaper>>> playlist;
  int playStaticIndex;
  int playDynamicIndex;

 public:
  bool status;
  bool signal;
  bool pause;
  std::string cmd;
  enum Mode { STATIC, DYNAMIC } mode;

  EngineContext(WallpaperManager &manager);
  void stop();
  void single();
  void next();
  void previous();
  void setMode(Mode mode);
  void changeMode();  // change to another mode
  std::shared_ptr<Wallpaper> getCurrent();
  std::shared_ptr<Wallpaper> getNext();
  std::shared_ptr<Wallpaper> getPrevious();
};

class Engine {
 private:
  EngineContext context;
  Communication comm;
  WallpaperManager manager;
  Renderer renderer;

  bool decodeStatic(std::shared_ptr<StaticWallpaper> wallpaper);
  bool decodeDynamic(std::shared_ptr<DynamicWallpaper> wallpaper);
  bool scale(Wallpaper &wallpaper);

  void handleCmd();

 public:
  Engine();
  void run();
};

}  // namespace Wow

#endif  // ENGINE_H

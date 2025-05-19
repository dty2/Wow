#include "Engine.h"

#include "Communication.h"
#include "Renderer.h"
#include "WallPaper.h"

namespace Wow {

int& EngineContext::getIntervalTime() { return info.intervalTime; }

std::pair<bool, bool> EngineContext::getAutoChange() {
  return std::pair<bool, bool>(
      manager.getConfiguration(WallPaper::STATIC).autoChange,
      manager.getConfiguration(WallPaper::DYNAMIC).autoChange);
}

Status EngineContext::getStatus() { return status; }

WallPaper::Type EngineContext::getMode() { return mode; }

std::string EngineContext::getPlayList() { return playlist.getName(); }

void EngineContext::setPlayList(std::string playListName,
                                WallPaper::Type mode) {
  this->mode = mode;
  auto config = manager.getConfiguration(mode);
  info.intervalTime = config.intervalTime;
  info.autoChange = config.autoChange;
  playlist = manager.getList(playListName, mode);
  info.index = 0;
}

WallPaperPtr EngineContext::currentWallPaper() { return playlist[info.index]; }

void EngineContext::toggleStop() {
  if (status == Status::STOP) {
    status = Status::PLAY;
  } else {
    status = Status::STOP;
  }
}

void EngineContext::next() {
  if (info.index == playlist.getSize() - 1) {
    info.index = 0;
    return;
  }
  info.index++;
  if (mode == WallPaper::DYNAMIC) {
    info.intervalTime =
        manager.getConfiguration(WallPaper::DYNAMIC).intervalTime;
  }
}

void EngineContext::previous() {
  if (info.index == 0) {
    info.index = playlist.getSize() - 1;
    return;
  }
  info.index--;
  if (mode == WallPaper::DYNAMIC) {
    info.intervalTime =
        manager.getConfiguration(WallPaper::DYNAMIC).intervalTime;
  }
}

EngineContext::EngineContext() {
  manager.manage();
  mode = manager.getStartMode();
  auto config = manager.getConfiguration(mode);
  status = config.autoChange == true ? Status::PLAY : Status::STOP;
  info.index = 0;
  info.listIndex = config.startList;
  info.intervalTime = config.intervalTime;
  playlist = manager.getList(info.listIndex, mode);
  signal = false;
}

void Engine::handleCommand() {
  while (!cmdList.empty()) {
    std::string cmd = cmdList.front();

    // Invaild cmd
    if (cmd == "help" || cmd == "version" || cmd == "error" ||
        cmd == "initlization" || cmd == "all") {
      cmdList.pop();
      return;
    }

    if (cmd == "stop") {
      context.toggleStop();
    } else if (cmd == "next") {
      context.next();
    } else if (cmd == "previous") {
      context.previous();
    } else {
      if (cmd.substr(0, 2) == "ls") {
        context.setPlayList(cmd.substr(2), WallPaper::STATIC);
      } else {
        context.setPlayList(cmd.substr(2), WallPaper::DYNAMIC);
      }
      play = context.getMode() == WallPaper::STATIC ? playStatic : playDynamic;
    }
    cmdList.pop();
  }
}

void Engine::start() {
  // Communication
  std::thread commThread([&] {
    while (true) {
      cmdList.push(communication.read());
      context.signal = true;
      if (cmdList.back() == "toggle") {
        break;
      }
    }
  });

  // Play: Decode and Render
  while (true) {
    WallPaperPtr wallPaperPtr = context.currentWallPaper();
    play(wallPaperPtr);

    if (context.signal) {
      // Handle Command
      if (cmdList.back() == "toggle") {
        break;
      } else {
        handleCommand();
        context.signal = false;
      }
    } else {
      // Auto Change
      if (context.getStatus() == Status::STOP) {
        continue;
      }

      if (context.getAutoChange().first &&
          context.getMode() == WallPaper::STATIC) {
        context.next();
      }

      if (context.getAutoChange().second &&
          context.getMode() == WallPaper::DYNAMIC) {
        if (context.getIntervalTime() == 0) {
          context.next();
        } else {
          context.getIntervalTime()--;
        }
      }
    }
  }

  commThread.join();
}

Engine::Engine()
    : communication(Communication::SERVER),
      decoder(context.signal),
      renderer(context.signal) {
  playStatic = [&](WallPaperPtr wallPaperPtr) {
    decoder.decode(wallPaperPtr);
    // Render must at main thread, sub thread can't use GPU.
    renderer.render(wallPaperPtr);

    int intervalTime = context.getIntervalTime();
    while (intervalTime--) {
      if (context.signal) break;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  };

  playDynamic = [&](WallPaperPtr wallPaperPtr) {
    std::thread decodeThread([&] { decoder.decode(wallPaperPtr); });
    // Render must at main thread, sub thread can't use GPU.
    renderer.render(wallPaperPtr);
    decodeThread.join();
  };

  play = context.getMode() == WallPaper::STATIC ? playStatic : playDynamic;
}

}  // namespace Wow

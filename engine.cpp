#include "engine.h"

#include "paper.h"
#include "renderer.h"

namespace Wow {

std::shared_ptr<Wallpaper> EngineContext::getCurrent() {
  if (mode == STATIC) {
    std::shared_ptr<StaticWallpaper> wallpaper =
        std::static_pointer_cast<StaticWallpaper>((*playlist)[playStaticIndex]);
    if (wallpaper->buffer.empty()) {
      std::cout << "error";
    } else {
      std::cout << "error";
    }
    return (*playlist)[playStaticIndex];
  } else {
    return (*playlist)[playDynamicIndex];
  }
}

std::shared_ptr<Wallpaper> EngineContext::getNext() {
  if (mode == STATIC) {
    if (playStaticIndex < playlist->size() - 1) {
      return (*playlist)[playStaticIndex + 1];
    } else {
      return (*playlist)[playlist->size() - 1];
    }
  } else if (mode == Mode::DYNAMIC) {
    if (playStaticIndex < playlist->size() - 1) {
      return (*playlist)[playDynamicIndex + 1];
    } else {
      return (*playlist)[playlist->size() - 1];
    }
  }
  return getCurrent();
}

std::shared_ptr<Wallpaper> EngineContext::getPrevious() {
  if (mode == STATIC) {
    if (playStaticIndex < 0) {
      return (*playlist)[playStaticIndex - 1];
    } else {
      return (*playlist)[0];
    }
  } else if (mode == Mode::DYNAMIC) {
    if (playStaticIndex > 0) {
      return (*playlist)[playDynamicIndex - 1];
    } else {
      return (*playlist)[0];
    }
  }
  return getCurrent();
}

void EngineContext::stop() { status = false; }

void EngineContext::single() { pause = true; }

void EngineContext::next() {
  if (mode == STATIC) {
    if (playStaticIndex == playlist->size() - 1) {
      playStaticIndex = 0;
    } else {
      playStaticIndex++;
    }
  } else if (mode == DYNAMIC) {
    if (playDynamicIndex == playlist->size() - 1) {
      playDynamicIndex = 0;
    } else {
      playDynamicIndex++;
    }
  }
}

void EngineContext::previous() {
  if (mode == STATIC) {
    if (playStaticIndex == 0) {
      playStaticIndex = playlist->size() - 1;
    } else {
      playStaticIndex--;
    }
  } else if (mode == DYNAMIC) {
    if (playDynamicIndex == 0) {
      playDynamicIndex = playlist->size() - 1;
    } else {
      playDynamicIndex--;
    }
  }
}

void EngineContext::setMode(Mode mode) {
  this->mode = mode;
  if (mode == Mode::STATIC) {
    playlist = std::make_shared<std::vector<std::shared_ptr<Wallpaper>>>(
        manager.staticWallpapers);
  } else {
    playlist = std::make_shared<std::vector<std::shared_ptr<Wallpaper>>>(
        manager.dynamicWallpapers);
  }
}

void EngineContext::changeMode() { setMode(mode == STATIC ? DYNAMIC : STATIC); }

EngineContext::EngineContext(WallpaperManager& manager) : manager(manager) {
  status = true;
  signal = false;
  pause = false;
  playStaticIndex = 0;
  playDynamicIndex = 0;
}

void Engine::handleCmd() {
  if (context.cmd == "stop") {
    context.stop();
  } else if (context.cmd == "next") {
    context.next();
  } else if (context.cmd == "previous") {
    context.previous();
  } else if (context.cmd == "mode") {
    context.changeMode();
  }
}

Engine::Engine()
    : comm(Communication::SERVER), context(manager), renderer(context.signal) {}

void Engine::run() {
  // Load
  std::pair<int, int> papers = manager.loadPapers();
  LOG(INFO) << "Static paper number:" << papers.first
            << " Dynamic paper number:" << papers.second;
  if (papers.first == 0) {
    LOG(WARNING)
        << "No static wallpaper to play, so play the dynamic wallpaper.";
    context.setMode(EngineContext::DYNAMIC);
    std::cout << "No static wallpaper to play, so play the dynamic wallpaper."
              << std::endl;
  } else if (papers.second == 0) {
    LOG(WARNING)
        << "No dynamic wallpaper to play, so play the static wallpaper.";
    context.setMode(EngineContext::STATIC);
    std::cout << "No dynamic wallpaper to play, so play the static wallpaper."
              << std::endl;
  } else if (papers.first != 0 && papers.second != 0) {
    context.setMode(EngineContext::STATIC);
  } else {
    LOG(WARNING) << "No wallpaper to play.";
    std::cout << "No wallpaper to play." << std::endl;
    return;
  }

  // Communication
  std::thread commThread([&] {
    while (context.status) {
      context.cmd = comm.read();
      context.signal = true;
      if (context.cmd == "stop") {
        break;
      }
    }
  });

  // Decode And Play
  while (context.status) {
    std::shared_ptr<Wallpaper> paper = context.getCurrent();

    preprocesser.decode(*paper);

    // Render must at main thread, sub thread can't use GPU.
    renderer.render(*paper);
    google::FlushLogFiles(google::INFO);
    google::FlushLogFiles(google::WARNING);
    google::FlushLogFiles(google::ERROR);

    if (context.signal) {
      handleCmd();
      context.signal = false;
      continue;
    }

    if (context.mode == EngineContext::STATIC) {
      if (!context.pause) {
        context.next();
      }
    }
  }

  commThread.join();
}

}  // namespace Wow

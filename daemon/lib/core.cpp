#include "core.h"

#include <memory>
#include <string>
#include <thread>

#include "common.h"
#include "log.hpp"
#include "resource/wallpaper.h"

void Core::handleAction(const Json& cmd) {
  auto action = cmd.value("action", "");
  auto target = cmd.value("target", "");

  Json result;
  if (action == "info") {
    std::string res = player->play_info() + resourcemanager->getResourceInfo();
    result["result"] = "Info:" + res;
    server->response(result);
    return;
  } else if (action == "list") {
    auto lists = resourcemanager->getWallPaperLists();

    if (!lists[target].size()) {
      result["result"] = target + " not exist / have no wallpapers.";
      server->response(result);
      return;
    }
  } else if (action == "test") {
    auto type = WallPaper::isWallPaper(target);
    if (type == wpType::NONE) {
      result["result"] = target + " can't be a Wallpaper";
      server->response(result);
      return;
    }
  }

  if (action == "refresh") resourcemanager->loadResource();
  if (action == "quit") active = false;

  tasklist.push(action, target);

  result["result"] = action + " successful.";
  server->response(result);
  player->notify();
}

bool Core::init(std::string workdir, std::string configdir) {
  server = std::make_unique<Server>(workdir, "wow.sock");
  if (!server->status) {
    LOG(INFO) << "Server init failed";
    return false;
  }

  // load resource
  resourcemanager = std::make_unique<ResourceManager>(configdir);
  if (!resourcemanager->loadResource()) return false;

  auto list = resourcemanager->getStartList();
  auto& lists = resourcemanager->getWallPaperLists();

  player = std::make_unique<Player>(tasklist, lists);

  if (!player->init(list)) return false;

  return true;
}

void Core::work() {
  std::thread control([this] {
    setThreadName("ipc");

    while (active) {
      Json cmd;

      if (!server->receive(cmd)) continue;
      if (cmd.is_null()) continue;

      handleAction(cmd);
    }
  });

  while (active) {
    player->play();
  }

  control.join();
}

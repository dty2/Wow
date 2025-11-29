#pragma once

#include <memory>

#include "ipc.h"
#include "player.h"
#include "resource/resourcemanager.h"

using Json = nlohmann::json;

class Core {
 private:
  std::unique_ptr<Server> server;
  std::unique_ptr<ResourceManager> resourcemanager;
  std::unique_ptr<Player> player;
  bool active = true;

  TaskList tasklist;

  void handleAction(const Json& cmd);

 public:
  bool init(std::string workdir, std::string configdir);
  void work();
};

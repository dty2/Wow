#include "Dashboard.h"

#include "Engine.h"
#include "Manager.h"

extern std::string VERSION;

namespace Wow {

Dashboard::Dashboard() : communication(Communication::CLIENT) {
  cmdMap.emplace("-h", std::bind(&Dashboard::help, this));

  cmdMap.emplace("-v", [&] {
    std::cout << "Wow Version: " << VERSION << "." << std::endl;
    if (!getStatus()) {
      return;
    }
    communication.write("07version");
  });

  cmdMap.emplace("-i", [&] {
    Manager manager;
    manager.init();
    std::cout << "Wow initlization success." << std::endl;
    if (!getStatus()) {
      return;
    }
    communication.write("12initlization");
  });

  cmdMap.emplace("-t", std::bind(&Dashboard::toggle, this));
  cmdMap.emplace("-s", [&] {
    if (!getStatus()) {
      std::cout << "Wow not run, run Wow -t first." << cmd << std::endl;
      return;
    }
    communication.write("04stop");
  });

  cmdMap.emplace("-n", [&] {
    if (!getStatus()) {
      std::cout << "Wow not run, run Wow -t first." << cmd << std::endl;
      return;
    }
    communication.write("04next");
  });

  cmdMap.emplace("-p", [&] {
    if (!getStatus()) {
      std::cout << "Wow not run, run Wow -t first." << cmd << std::endl;
      return;
    }
    communication.write("08previous");
  });

  cmdMap.emplace("-ls", [&] {
    if (!getStatus()) {
      std::cout << "Wow not run, run Wow -t first." << cmd << std::endl;
      return;
    }
    // FIXME: if cmd size > 100 will get error
    std::string controlCmd = "ls" + cmd2;
    std::string size = (controlCmd.size() < 10)
                           ? "0" + std::to_string(controlCmd.size())
                           : std::to_string(controlCmd.size());
    controlCmd = size + controlCmd;
    communication.write(controlCmd);
  });

  cmdMap.emplace("-ld", [&] {
    if (!getStatus()) {
      std::cout << "Wow not run, run Wow -t first." << cmd << std::endl;
      return;
    }

    // FIXME: if cmd size > 100 will get error
    std::string controlCmd = "ld" + cmd2;
    std::string size = (controlCmd.size() < 10)
                           ? "0" + std::to_string(controlCmd.size())
                           : std::to_string(controlCmd.size());
    controlCmd = size + controlCmd;
    communication.write(controlCmd);
  });

  cmdMap.emplace("-a", std::bind(&Dashboard::all, this));
}

bool Dashboard::getStatus() { return communication.status; }

void Dashboard::help() {
  std::cout << "Wow is a wallpaper player\n"
            << "Usage: Wow [options]\n"
            << "Options:\n"
            << "  -h            [H]elp\n"
            << "  -v            [V]ersion\n"
            << "  -i            [I]nitlization Wow\n"
            << "  -t            [T]oggle wallpaper\n"
            << "------------------------------------------------------------\n"
            << "  -s            Toggle of [S]top automatic play wallpaper\n"
            << "  -n            [N]ext wallpaper\n"
            << "  -p            [P]revious wallpaper\n"
            << "------------------------------------------------------------\n"
            << "  -ls ListName  Change to \"[L]istname\" of [S]tatic list\n"
            << "  -ld ListName  Change to \"[L]istname\" of [D]ynamic list\n"
            << "  -a            Show [A]ll your list\n";
  if (!getStatus()) {
    return;
  }
  communication.write("04help");
}

void Dashboard::toggle() {
  if (getStatus()) {
    communication.write("06toggle");
    return;
  }

  pid_t pid = fork();
  if (pid < 0) {
    LOG(ERROR) << "Fork failed";
    return;
  }

  // Child process
  if (pid == 0) {
    // if (setsid() < 0) {
    //   LOG(ERROR) << "Setsid failed";
    //   return;
    // }

    Engine engine;
    engine.start();
  }
}

void Dashboard::list() {}

void Dashboard::all() {
  int listCount = 0;
  Manager manager;
  manager.manage();
  std::cout << "-------- Static ----------\n";
  for (auto item : manager.getLists(WallPaper::STATIC)) {
    std::cout << listCount << ": " << item.first << "\n";
    listCount++;
  }

  listCount = 0;
  std::cout << "-------- Dynamic ---------\n";
  for (auto item : manager.getLists(WallPaper::DYNAMIC)) {
    std::cout << listCount << ": " << item.first << "\n";
    listCount++;
  }
  if (!getStatus()) {
    return;
  }
  communication.write("03all");
}

void Dashboard::parseCmd(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "No option provided, Use -h for help." << std::endl;
    return;
  } else if (argc == 2) {
    cmd = argv[1];
  } else {
    cmd = argv[1];
    if (cmd == "-ls" || cmd == "-ld") {
      cmd2 = argv[2];
    } else {
      std::cout << "Command format error, Use -h for help." << std::endl;
    }
  }
}

void Dashboard::run() {
  if (cmdMap[cmd]) {
    cmdMap[cmd]();
  } else {
    std::cout << "Wow don't have cmd: " << cmd << std::endl;
    communication.write("05error");
  }
}

}  // namespace Wow

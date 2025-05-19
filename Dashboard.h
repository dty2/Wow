#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "Common.h"
#include "Communication.h"

namespace Wow {

class Dashboard {
  Communication communication;
  std::string cmd, cmd2;
  std::unordered_map<std::string, std::function<void()>> cmdMap;
  bool getStatus();

  // Function
  void help();
  void toggle();
  void mode();
  void list();
  void all();

 public:
  Dashboard();
  void parseCmd(int argc, char *argv[]);
  void run();
};

}  // namespace Wow
#endif  // DASHBOARD_H

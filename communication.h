#ifndef COMMUNICATION_H
#define COMMUNICATION_H

extern "C" {
#include <sys/un.h>
}

#include "common.h"

namespace Wow {

class Communication {
  std::string workDir = getenv("HOME") + std::string("/.config/Wow");
  std::string sock = workDir + "/Wow.sock";
  sockaddr_un addr;
  int socket;

  void initClient();
  void initServer();

 public:
  enum Type { CLIENT, SERVER } type;
  Communication(Type type);
  ~Communication();
  bool status;

  std::string read();
  void write(std::string cmd);
};

}  // namespace Wow

#endif  // COMMUNICATION_H

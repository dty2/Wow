#ifndef COMMUNICATION_H
#define COMMUNICATION_H

extern "C" {
#include <sys/un.h>
}

#include "Common.h"

namespace Wow {

class Communication {
  const int ControlHead = 4;
  const int ControlBody = 32;
  std::string workDir = std::string("/tmp/Wow");
  std::string sockFile = workDir + "/Wow.sock";
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

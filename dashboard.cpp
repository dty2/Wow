#include "dashboard.h"

#include "communication.h"

namespace Wow {

bool Dashboard::getStatus() {
  Communication comm(Communication::CLIENT);
  if (comm.status) {
    comm.write("06status");
  }
  return comm.status;
}

void Dashboard::control(std::string cmd) {
  Communication comm(Communication::CLIENT);
  if (cmd == "-t") {
    comm.write("04stop");
  } else if (cmd == "-n") {
    comm.write("04next");
  } else if (cmd == "-p") {
    comm.write("08previous");
  } else if (cmd == "-m") {
    comm.write("04mode");
  } else if (cmd == "-s") {
    comm.write("06single");
  } else {
    std::cout << "Wow not have cmd: " << cmd << std::endl;
  }
}

}  // namespace Wow

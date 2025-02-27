#include "communication.h"

extern "C" {
#include <sys/socket.h>
#include <unistd.h>
}

namespace Wow {

Communication::Communication(Type type) : type(type) {
  LOG(INFO) << "Open communication";
  if ((socket = ::socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    LOG(ERROR) << "Get socket failed.";
    return;
  }
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, sock.c_str(), sizeof(addr.sun_path) - 1);

  if (type == CLIENT) {
    initClient();
  } else {
    initServer();
  }
}

void Communication::initClient() {
  if (::connect(socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    status = false;
    LOG(WARNING) << "Socket connect failed.";
    return;
  }
  status = true;
  LOG(INFO) << "Socket connect success.";
}

void Communication::initServer() {
  unlink(sock.c_str());
  if (::bind(this->socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    LOG(ERROR) << "Failed to bind socket: " << strerror(errno);
    close(socket);
    return;
  }

  // The max size that client connect is 1
  if (::listen(this->socket, 1) < 0) {
    LOG(ERROR) << "Failed to listen on socket: " << strerror(errno);
    close(socket);
    return;
  }

  LOG(INFO) << "Server listening on " << sock;
}

Communication::~Communication() {
  if (type == SERVER) {
    unlink(sock.c_str());
  }
  close(socket);
  LOG(INFO) << "Close communication";
}

std::string Communication::read() {
  int socket_;
  if ((socket_ = accept(socket, nullptr, nullptr)) < 0) {
    LOG(ERROR) << "Failed to accept connection: " << strerror(errno);
  }

  std::string cmdSize(2, '\0');
  if (::read(socket_, cmdSize.data(), 2) <= 0) {
    LOG(ERROR) << "Read error: " << strerror(errno)
               << " Read cmd size is: " << cmdSize;
  }

  std::string cmd(std::stoi(cmdSize), '\0');

  if (::read(socket_, cmd.data(), std::stoi(cmdSize)) > 0) {
    LOG(INFO) << "Read cmd is: " << cmd;
  } else {
    LOG(ERROR) << "Read error: " << strerror(errno);
  }
  close(socket_);
  return cmd;
}

void Communication::write(std::string cmd) {
  if (::write(socket, cmd.c_str(), cmd.length()) <= 0) {
    LOG(ERROR) << "Write error: " << strerror(errno);
  }
  LOG(INFO) << "Write cmd is: " << cmd;
}

}  // namespace Wow

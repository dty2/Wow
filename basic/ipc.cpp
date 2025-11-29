#include "ipc.h"

#include <fcntl.h>

#include <string>
#include <string_view>

#include "common.h"
#include "log.hpp"

extern "C" {
#include <sys/socket.h>
#include <unistd.h>
}

Communication::Communication(std::string workdir, std::string sockfile)
    : WorkDir(workdir), SockFile(workdir + sockfile) {
  if ((socket = ::socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    LOG(ERROR) << "Get socket failed.";
    status = false;
    return;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SockFile.data(), sizeof(addr.sun_path) - 1);
  status = true;
}

Communication::~Communication() {
  if (socket > 0) {
    ::close(socket);
  }

  LOG(INFO) << "Close communication";
}

Client::Client(std::string workdir, std::string sockfile)
    : Communication(workdir, sockfile) {
  if (!status) return;

  if (::connect(socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    status = false;
    LOG(WARNING) << "Socket connect failed, error: " << strerror(errno);
    return;
  }

  status = true;
  LOG(INFO) << "Socket connect success.";
}

bool Client::write(std::string_view target) {
  if (::write(socket, target.data(), target.length()) <= 0) {
    LOG(ERROR) << "write error: " << strerror(errno);
    return false;
  }
  return true;
}

bool Client::read(std::string& ss) {
  while (true) {
    char buf[50];
    int status = ::read(socket, buf, 50);

    if (status == 0) {
      LOG(INFO) << "read finish";
      break;
    } else if (status < 0) {
      LOG(ERROR) << "Read error: " << strerror(errno);
      return false;
    } else {
      ss.append(buf, status);
    }
  }

  return true;
}

bool Client::request(const Json& cmd) {
  this->write(cmd.dump());
  shutdown(socket, SHUT_WR);
  return true;
}

bool Client::receive(Json& json) {
  std::string ss;
  if (!this->read(ss)) {
    return false;
  }

  json = Json::parse(ss, nullptr, false /*No Exception*/);

  if (json.is_null()) {
    LOG(ERROR) << "JSON parse failed\n";
    return false;
  }

  return true;
}

Server::Server(std::string workdir, std::string sockfile)
    : Communication(workdir, sockfile) {
  if (!exists(WorkDir)) {
    create_directory(WorkDir);
  }

  // The previous local socket file existed, so it will be deleted here.
  if (exists(SockFile)) {
    remove(SockFile);
  }

  if (::bind(this->socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    LOG(ERROR) << "Failed to bind socket: " << strerror(errno);
    return;
  }

  // The max size that client connect is 1
  if (::listen(this->socket, 1) < 0) {
    LOG(ERROR) << "Failed to listen on socket: " << strerror(errno);
    return;
  }

  LOG(INFO) << "Server listening on " << SockFile;
}

bool Server::read(std::string& ss) {
  if ((socket_ = accept(socket, nullptr, nullptr)) < 0) {
    LOG(WARNING) << "Failed to accept connection: " << strerror(errno);
    if (!exists(SockFile)) {
      return false;
    }
    return true;
  }

  while (true) {
    char buf[50];
    int status = ::read(socket_, buf, 50);
    if (status == 0) {
      LOG(INFO) << "read finish";
      break;
    } else if (status < 0) {
      LOG(ERROR) << "Read error: " << strerror(errno);
      return false;
    } else {
      ss.append(buf, status);
    }
  }

  return true;
}

bool Server::write(std::string_view target) {
  if (::write(socket_, target.data(), target.length()) <= 0) {
    LOG(ERROR) << "write error: " << strerror(errno);
    return false;
  }

  return true;
};

bool Server::receive(Json& json) {
  std::string ss;

  if (!this->read(ss)) {
    return false;
  }

  json = Json::parse(ss, nullptr, false /*No Exception*/);

  if (json.is_null()) {
    LOG(ERROR) << "JSON parse failed\n";
    return false;
  }

  return true;
}

bool Server::response(const Json& res) {
  this->write(res.dump());
  ::close(socket_);
  return true;
}

Server::~Server() {
  if (socket > 0) {
    ::shutdown(socket, SHUT_RDWR);
    ::close(socket);
    socket = -1;
  }

  remove(SockFile);
  LOG(INFO) << "Remove socket file";
}

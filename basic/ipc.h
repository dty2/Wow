#pragma once

#define JSON_NOEXCEPTION

#include <nlohmann/json.hpp>
#include <string_view>

extern "C" {
#include <sys/un.h>
#include <unistd.h>
}

using Json = nlohmann::json;

class Communication {
 protected:
  std::string WorkDir;
  std::string SockFile;
  sockaddr_un addr;
  int socket = -1;

  virtual bool read(std::string& ss) = 0;
  virtual bool write(std::string_view target) = 0;

 public:
  Communication(std::string WorkDir, std::string SockFile);
  virtual ~Communication();
  bool status = false;
};

class Client : public Communication {
 private:
  bool read(std::string& ss) override;
  bool write(std::string_view target) override;

 public:
  Client(std::string WorkDir, std::string SockFile);
  bool request(const Json& cmd);
  bool receive(Json& json);
};

class Server : public Communication {
 private:
  int socket_ = -1;
  bool read(std::string& ss) override;
  bool write(std::string_view target) override;

 public:
  Server(std::string WorkDir, std::string SockFile);
  ~Server();
  bool receive(Json& json);
  bool response(const Json& res);
};

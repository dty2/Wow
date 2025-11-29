#include "common.h"

#include <unistd.h>

#include <filesystem>
#include <system_error>

#include "log.hpp"

void setThreadName(std::string name) {
  int ret = pthread_setname_np(pthread_self(), name.c_str());
  if (ret != 0) {
    LOG(ERROR) << "PThread setname failed.";
    return;
  }
}

bool create_directory(std::string path) {
  std::error_code ec;
  bool status = std::filesystem::create_directory(path, ec);
  if (ec) {
    LOG(ERROR) << ec.message();
    return false;
  }

  return status;
}

bool is_directory(std::string path) {
  std::error_code ec;
  bool status = std::filesystem::is_directory(path, ec);
  if (ec) {
    LOG(ERROR) << ec.message();
    return false;
  }

  return status;
}

bool exists(std::string path) {
  std::error_code ec;
  bool status = std::filesystem::exists(path, ec);
  if (ec) {
    LOG(ERROR) << ec.message();
    return false;
  }

  return status;
}

bool remove(std::string path) {
  std::error_code ec;
  bool status = std::filesystem::remove(path, ec);
  if (ec) {
    LOG(ERROR) << ec.message();
    return false;
  }

  return status;
}

bool setWorkDir(std::string& workdir, Source source) {
  if (source == Source::DEFALUT) {
    if (workdir.empty())
      workdir = "/tmp/wow/";
    else
      workdir += "/wow/";
  } else {
    if (::access(workdir.c_str(), F_OK | R_OK | W_OK) != 0) {
      std::cerr << workdir << " can't use." << std::endl;
      return false;
    }

    if (!is_directory(workdir)) {
      std::cerr << workdir << " is not a directory." << std::endl;
      return false;
    }

    if (workdir.back() != '/') {
      workdir += '/';
    }

    workdir += "wow/";
  }
  return true;
}

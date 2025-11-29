#include <signal.h>

#include <cstdlib>
#include <filesystem>
#include <ostream>
#include <string>

#include "common.h"
#include "core.h"
#include "log.hpp"
#include "version.inc"

std::string PidFile = "wow.pid";

Option<bool> Help({"help", "h"}, "get help", false);
Option<bool> Version({"version", "v"}, "get version", false);
Option<std::string> FileLog({"file-log", ""}, "Set log type(FILE/STDIO)",
                            "STDIO");

const char* runtimeDir = getenv("XDG_RUNTIME_DIR");
Option<std::string> WorkDir({"work-dir", ""}, "Set work directory",
                            runtimeDir ? std::string(runtimeDir)
                                       : std::string());

const char* configDir = getenv("XDG_CONFIG_HOME");
Option<std::string> ConfigDir({"config-dir", ""}, "Set config directory",
                              configDir ? std::string(configDir)
                                        : std::string());

std::unique_ptr<Core> core;

// if not running, it will create pid file, and runing.
// if running, it will return ture.
bool isRunning() {
  if (exists(PidFile)) {
    std::ifstream ifs(PidFile);
    if (!ifs.is_open()) {
      std::cerr << "can't open pid file " << PidFile << " : "
                << std::strerror(errno);
      return true;
    }

    int pid;
    ifs >> pid;

    if (pid > 0 && ::kill(pid, 0) == 0) {
      return true;
    }
  }

  std::ofstream ofs(PidFile);
  if (!ofs.is_open()) {
    std::cerr << "can't open pid file " << PidFile << " : "
              << std::strerror(errno);
    return true;
  }
  ofs << getpid() << std::endl;
  ofs.close();

  return false;
}

int main(int argc, char* argv[]) {
  OptionTable& options = parseOptions(argc, argv);

  std::string workdir = WorkDir.getValue();
  if (!setWorkDir(workdir, WorkDir.getSource())) {
    return 0;
  }

  std::string LogPath = workdir + "log/daemon.txt";
  PidFile = workdir + PidFile;

  if (!std::filesystem::exists(workdir)) {
    std::filesystem::create_directory(workdir);
  }

  if (isRunning()) {
    std::cerr << "wow-daemon is runing now, can't run again." << std::endl;
    return 0;
  }

  // If the user sets the log to a file format, the log will be saved to a file.
  // Otherwise, it depends on whether the user is in DEBUG mode.
  OutPutType logtype;
  if (FileLog.getSource() == Source::CLI) {
    if (FileLog.getValue() == "STDIO")
      logtype = OutPutType::STDIO;
    else
      logtype = OutPutType::LOGFILE;
  } else {
#ifdef NDEBUG
    logtype = OutPutType::LOGFILE;
#else
    logtype = OutPutType::STDIO;
#endif
  }

  if (!Logger::initLog(INFO, logtype, LogPath)) {
    return 0;
  }

  if (Help) {
    printHelp();
    return 0;
  }

  if (Version) {
    std::cerr << VERSION << std::endl;
    return 0;
  }

  core = std::make_unique<Core>();

  std::string configdir = ConfigDir.getValue();
  if (ConfigDir.getSource() == Source::DEFALUT) {
    if (configdir.empty()) {
      const char* home = getenv("HOME");
      if (exists(std::string(home) + "/.config/")) {
        configdir = std::string(home) + "/.config/wow";
      } else {
        configdir = std::string(home) + "/.wow/";
      }
    }
  } else {
    if (::access(configdir.c_str(), F_OK | R_OK | W_OK) != 0) {
      std::cerr << configdir << " can't use." << std::endl;
    }

    if (!is_directory(configdir)) {
      std::cerr << configdir << " is not a directory." << std::endl;
    }
  }

  if (core->init(workdir, configdir)) {
    core->work();
  }

  remove(PidFile);
  Logger::releaseLog();
  return 0;
}

#include "common.h"
#include "ipc.h"
#include "log.hpp"
#include "version.inc"

Option<bool> Help({"help", "h"}, "[H]elp", false);
Option<bool> Version({"version", "v"}, "[V]ersion", false);

Option<bool> Info({"info", "i"}, "Show all [I]nfo", false);
Option<bool> Refresh({"refresh", "r"}, "[R]efresh", false);
Option<bool> Next({"next", "n"}, "[N]ext wallpaper", false);
Option<bool> Prev({"prev", "p"}, "[P]revious wallpaper", false);
Option<bool> Manual({"manual", "m"}, "[M]anual play", false);
Option<bool> Quit({"quit", "q"}, "[Q]uit wow", false);
Option<std::string> Test({"test", "t"}, "[T]est wallpaper", "");
Option<std::string> List({"list", "l"}, "Play [L]ist", "");

Option<std::string> FileLog({"file-log", ""}, "Set log type(FILE/STDIO)",
                            "STDIO");
Option<std::string> WorkDir({"work-dir", ""}, "Set work directory",
                            getenv("XDG_RUNTIME_DIR"));

void setRequest(Json& req, std::string action, std::string target) {
  req["action"] = action;
  req["target"] = target;
}

void control(std::string workDir, OptionTable options) {
  Client client(workDir, "wow.sock");

  if (!client.status) {
    if (Info) {
      std::cerr << "Daemon not work.\n";
      return;
    }

    std::cerr << "Can't find sock file" << workDir + "wow.sock\n";
    std::cerr << "Please run wow-daemon first.\n";
    return;
  }

  Json req;
  if (Info) {
    setRequest(req, "info", "");
  }

  if (Next) {
    setRequest(req, "next", "");
  }

  if (Prev) {
    setRequest(req, "prev", "");
  }

  if (Manual) {
    setRequest(req, "manual", "");
  }

  if (Refresh) {
    setRequest(req, "refresh", "");
  }

  if (Quit) {
    setRequest(req, "quit", "");
  }

  if (!Test.getValue().empty()) {
    setRequest(req, "test", Test.getValue());
  }

  if (!List.getValue().empty()) {
    setRequest(req, "list", List.getValue());
  }

  client.request(req);
  Json rec;
  client.receive(rec);
  std::cerr << rec["result"].get<std::string>() << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    printHelp();
  }

  OptionTable& options = parseOptions(argc, argv);

  std::string workdir = WorkDir.getValue();
  if (!setWorkDir(workdir, WorkDir.getSource())) {
    return 0;
  }

  std::string LogPath = workdir + "log/cli.txt";

  // If the user sets the log to a file format, the log will be saved to a
  // file. Otherwise, it depends on whether the user is in DEBUG mode.
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

  control(workdir, options);
  Logger::releaseLog();
  return 0;
}

#include "common.h"
#include "dashboard.h"
#include "engine.h"

void help() {
  std::cout << "Usage: Wow [options]\n"
            << "Options:\n"
            << "  -h               Help\n"
            << "  -t               Toggle wallpaper\n"
            << "  -n               Next wallpaper\n"
            << "  -p               Previous wallpaper\n"
            << "  -m               Change wallpaper to another "
               "mode(Static/Dynamic)\n";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "No command provided. Use -h for help." << std::endl;
    return 0;
  }

  google::InitGoogleLogging("WallPaper");
  FLAGS_minloglevel = google::ERROR;
  FLAGS_logtostderr = false;

  std::string_view cmd = argv[1];
  if (cmd == "-h") {
    help();
  }

  try {
    Wow::Dashboard dashboard;
    bool status = dashboard.getStatus();
    LOG(INFO) << status << cmd;

    if (!status && cmd == "-t") {
      Wow::Engine engine;
      engine.run();
    } else if (!status && cmd != "-t") {
      std::cout << "Wow is not running now. Use -t to run." << std::endl;
    } else {
      dashboard.control(cmd.data());
    }
  } catch (std::exception e) {
    LOG(ERROR) << e.what();
  }
  google::ShutdownGoogleLogging();

  return 0;
}

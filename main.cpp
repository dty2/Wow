#include "Common.h"
#include "Dashboard.h"

using namespace Wow;

std::string VERSION = "1.2.1";

int main(int argc, char *argv[]) {
  google::InitGoogleLogging("WallPaper");
  FLAGS_minloglevel = google::ERROR;
  FLAGS_logtostderr = false;

  try {
    Dashboard dashboard;
    dashboard.parseCmd(argc, argv);
    dashboard.run();
  } catch (std::exception e) {
    LOG(ERROR) << e.what();
  }

  google::ShutdownGoogleLogging();

  return 0;
}

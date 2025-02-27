#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "common.h"

namespace Wow {

class Dashboard {
 public:
  bool getStatus();
  void control(std::string cmd);
};

}  // namespace Wow
#endif  // DASHBOARD_H

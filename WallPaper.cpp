#include "WallPaper.h"

namespace Wow {

bool StaticWallPaper::isExtension(std::string e) { return extension[e]; }

bool DynamicWallPaper::isExtension(std::string e) { return extension[e]; }

}  // namespace Wow

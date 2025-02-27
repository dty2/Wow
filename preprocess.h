#ifndef PREPROCESS_H
#define PREPROCESS_H

#include "paper.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace Wow {

class Preprocess {
  bool decodeStatic(StaticWallpaper &wallpaper);
  bool decodeDynamic(DynamicWallpaper &wallpaper);

 public:
  void decode(Wallpaper &wallpaper);
  bool scale(Wallpaper &wallpaper);
};

}  // namespace Wow

#endif  // PREPROCESS_H

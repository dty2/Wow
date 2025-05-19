#ifndef DECODER_H
#define DECODER_H

#include "Common.h"
#include "WallPaper.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace Wow {

class Decoder {
 private:
  bool decodeStatic(StaticWallPaperPtr wallpaper);
  bool decodeDynamic(DynamicWallPaperPtr wallpaper);
  bool& signal;

 public:
  Decoder(bool& signal);
  void decode(WallPaperPtr wallPaperPtr);
};

}  // namespace Wow

#endif  // DECODER_H

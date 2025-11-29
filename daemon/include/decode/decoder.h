#pragma once

#include <atomic>
#include <memory>

#include "ffmpeg.h"
#include "resource/wallpaper.h"

class FrameBuffer;

class Decoder {
 private:
  std::atomic_bool& signal;
  FrameBuffer& frameBuffer;

  std::unique_ptr<FormatContext> formatContext;
  std::unique_ptr<CodecContext> codecContext;
  int videoStreamIndex;
  WallPaper* wallpaper;

  bool getStream();
  bool getDecoder();
  bool handleFrame(std::shared_ptr<Frame> frame);
  bool decodeFrame(std::shared_ptr<Frame> frame);

 public:
  bool decodeFinished = false;

  bool prepareDecoder(WallPaper* wallpaper);
  void setWallPaperInfo();
  bool decode();
  Decoder(std::atomic_bool& signal, FrameBuffer& frameBuffer);
};

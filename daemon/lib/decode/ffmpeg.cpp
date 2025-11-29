#include "decode/ffmpeg.h"

#include "log.hpp"

FormatContext::FormatContext(const std::string& path) {
  formatContext = nullptr;
  if (avformat_open_input(&formatContext, path.c_str(), nullptr, nullptr)) {
    LOG(ERROR) << "Could not open input file: " << path;
    formatContext = nullptr;
    return;
  }
}

FormatContext::~FormatContext() { avformat_close_input(&formatContext); }

CodecContext::CodecContext(const AVCodec* codec) {
  codecContext = avcodec_alloc_context3(codec);

  if (!codecContext) {
    LOG(ERROR) << "Could not allocate codec context.";
    codecContext = nullptr;
    return;
  }
}

CodecContext::~CodecContext() { avcodec_free_context(&codecContext); }

Packet::Packet() {
  packet = av_packet_alloc();
  if (!packet) {
    LOG(ERROR) << "Could not allocate packet .";
    packet = nullptr;
    return;
  }
}

Packet::~Packet() { av_packet_free(&packet); }

Frame::Frame() {
  frame = av_frame_alloc();
  if (!frame) {
    LOG(ERROR) << "Could not allocate frame .";
    frame = nullptr;
    return;
  }
}

Frame::~Frame() { av_frame_free(&frame); }

ScaleContext::ScaleContext(AVFrame* frame) {
  sws_ctx = sws_getContext(
      frame->width, frame->height, (AVPixelFormat)frame->format, frame->width,
      frame->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
  if (!sws_ctx) {
    LOG(ERROR) << "Could not initialize the conversion context.";
    sws_ctx = nullptr;
    return;
  }
}

ScaleContext::~ScaleContext() { sws_freeContext(sws_ctx); }

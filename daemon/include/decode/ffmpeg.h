#pragma once

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class FormatContext {
 public:
  FormatContext(const std::string& path);
  ~FormatContext();

  FormatContext(const FormatContext&) = delete;
  FormatContext& operator=(const FormatContext&) = delete;
  FormatContext(FormatContext&&) noexcept = default;
  FormatContext& operator=(FormatContext&&) noexcept = default;

  operator AVFormatContext*() const { return formatContext; }
  AVFormatContext* operator->() const { return formatContext; }

 private:
  AVFormatContext* formatContext;
};

class CodecContext {
 public:
  CodecContext(const AVCodec* codec);
  ~CodecContext();

  CodecContext(const CodecContext&) = delete;
  CodecContext& operator=(const CodecContext&) = delete;
  CodecContext(CodecContext&&) noexcept = default;
  CodecContext& operator=(CodecContext&&) noexcept = default;

  operator AVCodecContext*() const { return codecContext; }
  AVCodecContext* operator->() const { return codecContext; }

 private:
  AVCodecContext* codecContext;
};

class Packet {
 public:
  Packet();
  ~Packet();
  Packet(const Packet&) = delete;
  Packet& operator=(const Packet&) = delete;
  Packet(Packet&&) noexcept = default;
  Packet& operator=(Packet&&) noexcept = default;
  operator AVPacket*() const { return packet; }
  AVPacket* operator->() const { return packet; }

 private:
  AVPacket* packet;
};

class Frame {
 public:
  Frame();
  ~Frame();
  Frame(const Frame&) = delete;
  Frame& operator=(const Frame&) = delete;
  Frame(Frame&&) noexcept = default;
  Frame& operator=(Frame&&) noexcept = default;
  operator AVFrame*() const { return frame; }
  AVFrame* operator->() const { return frame; }

 private:
  AVFrame* frame;
};

class ScaleContext {
 public:
  ScaleContext(AVFrame* frame);
  ~ScaleContext();
  ScaleContext(const ScaleContext&) = delete;
  ScaleContext& operator=(const ScaleContext&) = delete;
  ScaleContext(ScaleContext&&) noexcept = default;
  ScaleContext& operator=(ScaleContext&&) noexcept = default;
  operator SwsContext*() const { return sws_ctx; }
  SwsContext* operator->() const { return sws_ctx; }

 private:
  SwsContext* sws_ctx;
};

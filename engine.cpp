#include "engine.h"

#include "paper.h"
#include "renderer.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace Wow {

std::shared_ptr<Wallpaper> EngineContext::getCurrent() {
  if (mode == STATIC) {
    std::shared_ptr<StaticWallpaper> wallpaper =
        std::static_pointer_cast<StaticWallpaper>((*playlist)[playStaticIndex]);
    return (*playlist)[playStaticIndex];
  } else {
    return (*playlist)[playDynamicIndex];
  }
}

void EngineContext::stop() { status = false; }

void EngineContext::single() { pause = true; }

void EngineContext::next() {
  if (mode == STATIC) {
    if (playStaticIndex == playlist->size() - 1) {
      playStaticIndex = 0;
    } else {
      playStaticIndex++;
    }
  } else if (mode == DYNAMIC) {
    if (playDynamicIndex == playlist->size() - 1) {
      playDynamicIndex = 0;
    } else {
      playDynamicIndex++;
    }
  }
}

void EngineContext::previous() {
  if (mode == STATIC) {
    if (playStaticIndex == 0) {
      playStaticIndex = playlist->size() - 1;
    } else {
      playStaticIndex--;
    }
  } else if (mode == DYNAMIC) {
    if (playDynamicIndex == 0) {
      playDynamicIndex = playlist->size() - 1;
    } else {
      playDynamicIndex--;
    }
  }
}

void EngineContext::setMode(Mode mode) {
  this->mode = mode;
  if (mode == Mode::STATIC) {
    playlist = std::make_shared<std::vector<std::shared_ptr<Wallpaper>>>(
        manager.staticWallpapers);
  } else {
    playlist = std::make_shared<std::vector<std::shared_ptr<Wallpaper>>>(
        manager.dynamicWallpapers);
  }
}

bool EngineContext::initMode() {
  if (papers.first == 0 && papers.second != 0) {
    setMode(EngineContext::DYNAMIC);
    std::cout << "No static wallpaper to play, so play the dynamic wallpaper."
              << std::endl;
  } else if (papers.first != 0 && papers.second == 0) {
    setMode(EngineContext::STATIC);
    std::cout << "No dynamic wallpaper to play, so play the static wallpaper."
              << std::endl;
  } else if (papers.first != 0 && papers.second != 0) {
    setMode(EngineContext::STATIC);
    std::cout << "Default mode is static, so play the static wallpaper"
              << std::endl;
  } else {
    std::cout << "No wallpaper to play. \n"
              << "Please place your wallpaper \"example.png/jpg/webp/...\" on "
              << "$HOME/.config/Wow/static. \n"
              << "Or place your wallpaper \"example.mp4/webm/...\" on "
              << "$HOME/.config/Wow/dynamic. \n"
              << "Then run \"./Wow -t\" again. \n";
    return false;
  }
  return true;
}

void EngineContext::changeMode() {
  if (papers.second != 0 && mode == STATIC) {
    setMode(DYNAMIC);
  } else if (papers.first != 0 && mode == DYNAMIC) {
    setMode(STATIC);
  } else {
    std::cout << "Don't have Another mode wallpaper, change mode fault"
              << std::endl;
  }
}

EngineContext::EngineContext(WallpaperManager& manager) : manager(manager) {
  status = true;
  signal = false;
  pause = false;
  mode = Mode::STATIC;
  playStaticIndex = 0;
  playDynamicIndex = 0;
  LOG(INFO) << "Static paper number:" << papers.first
            << " Dynamic paper number:" << papers.second;
}

void Engine::handleCmd() {
  if (context.cmd == "stop") {
    context.stop();
  } else if (context.cmd == "next") {
    context.next();
  } else if (context.cmd == "previous") {
    context.previous();
  } else if (context.cmd == "mode") {
    context.changeMode();
  }
}

Engine::Engine()
    : comm(Communication::SERVER), context(manager), renderer(context.signal) {}

void Engine::run() {
  // Load
  context.papers = manager.loadPapers();
  if (!context.initMode()) {
    return;
  }

  // Communication
  std::thread commThread([&] {
    while (context.status) {
      context.cmd = comm.read();
      context.signal = true;
      if (context.cmd == "stop") {
        break;
      }
    }
  });

  // Decode And Play
  while (context.status) {
    std::shared_ptr<Wallpaper> paper = context.getCurrent();

    if (context.mode == EngineContext::STATIC) {
      decodeStatic(static_pointer_cast<StaticWallpaper>(paper));
      // Render must at main thread, sub thread can't use GPU.
      renderer.render(*paper);
    } else {
      std::thread decodeThread(
          &Engine::decodeDynamic, this,
          std::static_pointer_cast<DynamicWallpaper>(paper));
      renderer.render(*paper);
      decodeThread.join();
    }

    google::FlushLogFiles(google::INFO);
    google::FlushLogFiles(google::WARNING);
    google::FlushLogFiles(google::ERROR);

    if (context.signal) {
      handleCmd();
      context.signal = false;
      continue;
    }

    if (context.mode == EngineContext::STATIC) {
      if (!context.pause) {
        context.next();
      }
    }
  }

  commThread.join();
}

bool Engine::decodeStatic(std::shared_ptr<StaticWallpaper> wallpaper) {
  LOG(INFO) << "Decode wallpaper: " << wallpaper->name;

  // Allocate and open the input file
  AVFormatContext* formatContext = avformat_alloc_context();
  if (avformat_open_input(&formatContext, wallpaper->path.c_str(), nullptr,
                          nullptr) != 0) {
    LOG(ERROR) << "Could not open input file: " << wallpaper->name;
    return false;
  }

  if (avformat_find_stream_info(formatContext, nullptr) < 0) {
    LOG(ERROR) << "Could not find stream information for file: "
               << wallpaper->name;
    avformat_close_input(&formatContext);
    return false;
  }

  // Find the first video stream
  int videoStreamIndex = -1;
  AVCodecParameters* codecpar = nullptr;
  for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
    if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      codecpar = formatContext->streams[i]->codecpar;
      videoStreamIndex = i;
      break;
    }
  }
  if (codecpar == nullptr) {
    LOG(ERROR) << "Could not find valid video stream in file: "
               << wallpaper->name;
    avformat_close_input(&formatContext);
    return false;
  }

  // Select the decoder based on the video stream parameters
  const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
  if (codec == nullptr) {
    LOG(ERROR) << "Could not find suitable codec.";
    avformat_close_input(&formatContext);
    return false;
  }

  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
  if (codecContext == nullptr) {
    LOG(ERROR) << "Could not allocate codec context.";
    avformat_close_input(&formatContext);
    return false;
  }

  if (avcodec_parameters_to_context(codecContext, codecpar) < 0) {
    LOG(ERROR) << "Could not copy codec parameters to context for file: "
               << wallpaper->name;
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return false;
  }

  if (avcodec_open2(codecContext, codec, nullptr) < 0) {
    LOG(ERROR) << "Could not open codec.";
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return false;
  }

  // Allocate packet and frame
  AVPacket* packet = av_packet_alloc();
  AVFrame* frame = av_frame_alloc();
  if (!packet || !frame) {
    LOG(ERROR) << "Could not allocate packet or frame.";
    if (packet) av_packet_free(&packet);
    if (frame) av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return false;
  }

  // Read frame data (only decode the first frame)
  int response = 0;
  bool gotFrame = false;
  while (av_read_frame(formatContext, packet) >= 0 && !gotFrame) {
    if (packet->stream_index == videoStreamIndex) {
      response = avcodec_send_packet(codecContext, packet);
      if (response < 0) {
        LOG(ERROR) << "Error while sending packet to decoder.";
        break;
      }
      response = avcodec_receive_frame(codecContext, frame);
      if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
        // No frame data is obtained immediately, continue processing
      } else if (response < 0) {
        LOG(ERROR) << "Error while receiving frame from decoder.";
        break;
      } else {
        gotFrame = true;
      }
    }
    av_packet_unref(packet);
  }
  if (!gotFrame) {
    LOG(ERROR) << "No frame decoded from file: " << wallpaper->name;
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return false;
  }

  // Convert the decoded frame to RGB24 format using sws_scale
  struct SwsContext* sws_ctx = sws_getContext(
      frame->width, frame->height, (AVPixelFormat)frame->format, frame->width,
      frame->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
  if (!sws_ctx) {
    LOG(ERROR) << "Could not initialize the conversion context.";
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return false;
  }

  AVFrame* rgb_frame = av_frame_alloc();
  rgb_frame->format = AV_PIX_FMT_RGB24;
  rgb_frame->width = frame->width;
  rgb_frame->height = frame->height;
  if (av_frame_get_buffer(rgb_frame, 0) < 0) {
    LOG(ERROR) << "Could not allocate buffer for converted frame.";
    sws_freeContext(sws_ctx);
    av_frame_free(&rgb_frame);
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return false;
  }

  sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
            rgb_frame->data, rgb_frame->linesize);

  // Save the converted image data to wallpaper
  wallpaper->size.width = rgb_frame->width;
  wallpaper->size.height = rgb_frame->height;
  int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, rgb_frame->width,
                                           rgb_frame->height, 1);
  wallpaper->buffer.resize(num_bytes);
  av_image_copy_to_buffer(wallpaper->buffer.data(), num_bytes, rgb_frame->data,
                          rgb_frame->linesize, AV_PIX_FMT_RGB24,
                          rgb_frame->width, rgb_frame->height, 1);

  // free
  sws_freeContext(sws_ctx);
  av_frame_free(&rgb_frame);
  av_packet_free(&packet);
  av_frame_free(&frame);
  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);

  return true;
}

bool Engine::decodeDynamic(std::shared_ptr<DynamicWallpaper> wallpaper) {
  LOG(INFO) << "Decode dynamic wallpaper: " << wallpaper->name;

  // Find the format info
  AVFormatContext* formatContext = nullptr;
  if (avformat_open_input(&formatContext, wallpaper->path.c_str(), nullptr,
                          nullptr) != 0) {
    LOG(ERROR) << "Can't open the input file: " << wallpaper->name;
    return false;
  }

  if (avformat_find_stream_info(formatContext, nullptr) < 0) {
    LOG(ERROR) << "Can't find the stream info: " << wallpaper->name;
    avformat_close_input(&formatContext);
    return false;
  }

  // Find the video stream info
  int videoStreamIndex = -1;
  for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
    if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStreamIndex = i;
      break;
    }
  }
  if (videoStreamIndex == -1) {
    LOG(ERROR) << "Can't find the video stream: " << wallpaper->name;
    avformat_close_input(&formatContext);
    return false;
  }

  // Init the decoder
  AVCodecParameters* codecpar =
      formatContext->streams[videoStreamIndex]->codecpar;
  const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
  if (!codec) {
    LOG(ERROR) << "Can't find the decoder.";
    avformat_close_input(&formatContext);
    return false;
  }

  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
  if (!codecContext) {
    LOG(ERROR) << "Can't alloc the context of decoder.";
    avformat_close_input(&formatContext);
    return false;
  }

  if (avcodec_parameters_to_context(codecContext, codecpar) < 0) {
    LOG(ERROR) << "Can't copy the args of decoder to context.";
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return false;
  }

  if (avcodec_open2(codecContext, codec, nullptr) < 0) {
    LOG(ERROR) << "Can't open the decoder.";
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    return false;
  }

  // Get the frame rate
  AVStream* videoStream = formatContext->streams[videoStreamIndex];
  AVRational frame_rate =
      av_guess_frame_rate(formatContext, videoStream, nullptr);
  // Calculate the delay time each frame
  double frame_delay = av_q2d(av_inv_q(frame_rate));
  wallpaper->frameDelay = frame_delay * 1000;

  // Alloc the frame and packet
  AVFrame* frame = av_frame_alloc();
  AVPacket* packet = av_packet_alloc();

  // Init SwsContext and alloc the rgb frame
  struct SwsContext* sws_ctx = nullptr;
  AVFrame* rgb_frame = av_frame_alloc();

  int frameSize;
  // Read the frame
  while (av_read_frame(formatContext, packet) == 0) {
    if (packet->stream_index == videoStreamIndex) {
      if (avcodec_send_packet(codecContext, packet) != 0) {
        LOG(ERROR) << "decoder send packet failed.";
        break;
      }
      while (avcodec_receive_frame(codecContext, frame) == 0) {
        if (!sws_ctx) {
          // Init SwsContext
          sws_ctx =
              sws_getContext(frame->width, frame->height, codecContext->pix_fmt,
                             frame->width, frame->height, AV_PIX_FMT_RGB24,
                             SWS_BILINEAR, nullptr, nullptr, nullptr);
          if (!sws_ctx) {
            LOG(ERROR) << "Can't init sws_ctx.";
            goto cleanup;
          }

          // Alloc the rgb frame buffer
          rgb_frame->format = AV_PIX_FMT_RGB24;
          rgb_frame->width = frame->width;
          rgb_frame->height = frame->height;
          if (av_frame_get_buffer(rgb_frame, 0) < 0) {
            LOG(ERROR) << "Can't alloc the rgb frame buffer.";
            goto cleanup;
          }

          // Save the video size info
          wallpaper->size.width = frame->width;
          wallpaper->size.height = frame->height;
        }

        // Change the format to rgb24 from original format
        sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
                  rgb_frame->data, rgb_frame->linesize);

        // Calculate the rgb frame size
        int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, frame->width,
                                                 frame->height, 1);
        // Alloc the buffer and copy the data
        std::vector<uint8_t> buffer(num_bytes);
        av_image_copy_to_buffer(buffer.data(), num_bytes, rgb_frame->data,
                                rgb_frame->linesize, AV_PIX_FMT_RGB24,
                                frame->width, frame->height, 1);

        {
          std::lock_guard<std::mutex> lock(wallpaper->mtx);
          // Write the frame data to the buffer
          wallpaper->buffer.push(buffer);
          frameSize = wallpaper->buffer.size();
        }
        if (frameSize > 100) {
          std::this_thread::sleep_for(std::chrono::milliseconds(
              static_cast<int>(wallpaper->frameDelay * 90)));
        }
        if (context.signal) {
          {
            std::lock_guard<std::mutex> lock(wallpaper->mtx);
            while (!wallpaper->buffer.empty()) {
              wallpaper->buffer.pop();
            }
            if (wallpaper->buffer.empty()) {
              LOG(INFO) << "wallpaper " << wallpaper->name << " is empty";
            }
          }

          goto cleanup;
        }
      }
    }
    av_packet_unref(packet);
  }

  {
    std::lock_guard<std::mutex> lock(wallpaper->mtx);
    // Write the finish frame data to the buffer
    wallpaper->buffer.push({5, 0, 0, 0, 0, 0, 0, 0});
  }

cleanup:
  // Free
  av_frame_free(&rgb_frame);
  sws_freeContext(sws_ctx);
  av_packet_free(&packet);
  av_frame_free(&frame);
  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);

  return true;
}

}  // namespace Wow

#include "decode/decoder.h"

#include <thread>

#include "decode/ffmpeg.h"
#include "log.hpp"
#include "player.h"

bool Decoder::getStream() {
  formatContext = std::make_unique<FormatContext>(wallpaper->path);
  if (!(*formatContext)) {
    return false;
  }

  videoStreamIndex = -1;

  if (avformat_find_stream_info(*formatContext, nullptr) < 0) {
    LOG(ERROR) << "Could not find stream information for file: "
               << wallpaper->name;
    return false;
  }

  for (unsigned int i = 0; i < (*formatContext)->nb_streams; ++i) {
    AVCodecParameters* codecpar = (*formatContext)->streams[i]->codecpar;
    if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStreamIndex = i;
      break;
    }
  }

  if (videoStreamIndex == -1) {
    LOG(ERROR) << "Can't find the video stream: " << wallpaper->name;
    return false;
  }

  return true;
}

bool Decoder::getDecoder() {
  AVCodecParameters* codecpar =
      (*formatContext)->streams[videoStreamIndex]->codecpar;

  if (!codecpar) {
    LOG(ERROR) << "Could not find valid video stream in file: ";
    return false;
  }

  const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
  if (!codec) {
    LOG(ERROR) << "Could not find decoder.";
    return false;
  }
  codecContext = std::make_unique<CodecContext>(codec);
  if (!(*codecContext)) {
    return false;
  }

  if (avcodec_parameters_to_context(*codecContext, codecpar) < 0) {
    LOG(ERROR) << "Could not copy codec parameters to context for file: "
               << wallpaper->name;
    return false;
  }

  if (avcodec_open2(*codecContext, codec, nullptr) < 0) {
    LOG(ERROR) << "Could not open codec.";
    return false;
  }

  return true;
}

bool Decoder::handleFrame(std::shared_ptr<Frame> frame) {
  std::unique_ptr<Frame> rgb_frame = std::make_unique<Frame>();
  if (!(*rgb_frame)) {
    return false;
  }

  (*rgb_frame)->format = AV_PIX_FMT_RGB24;
  (*rgb_frame)->width = (*frame)->width;
  (*rgb_frame)->height = (*frame)->height;

  std::unique_ptr<ScaleContext> sws_ctx =
      std::make_unique<ScaleContext>(*frame);
  if (!(*sws_ctx)) {
    return false;
  }

  if (av_frame_get_buffer(*rgb_frame, 0) < 0) {
    LOG(ERROR) << "Could not allocate buffer for converted frame.";
    return false;
  }

  sws_scale(*sws_ctx, (*frame)->data, (*frame)->linesize, 0, (*frame)->height,
            (*rgb_frame)->data, (*rgb_frame)->linesize);

  int num_bytes = av_image_get_buffer_size(
      AV_PIX_FMT_RGB24, (*rgb_frame)->width, (*rgb_frame)->height, 1);

  oneFrame buffer;
  buffer.resize(num_bytes);
  av_image_copy_to_buffer(buffer.data(), num_bytes, (*rgb_frame)->data,
                          (*rgb_frame)->linesize, AV_PIX_FMT_RGB24,
                          (*rgb_frame)->width, (*rgb_frame)->height, 1);
  frameBuffer.push(std::move(buffer));

  return true;
}

bool Decoder::prepareDecoder(WallPaper* wallpaper) {
  this->wallpaper = wallpaper;
  return getStream() && getDecoder();
}

void Decoder::setWallPaperInfo() {
  AVCodecParameters* codecpar =
      (*formatContext)->streams[videoStreamIndex]->codecpar;

  wallpaper->info.width = codecpar->width;
  wallpaper->info.height = codecpar->height;

  // Get the frame rate
  AVRational frame_rate = av_guess_frame_rate(
      *formatContext, (*formatContext)->streams[videoStreamIndex], nullptr);

  // Calculate the delay time each frame
  double frame_delay = av_q2d(av_inv_q(frame_rate));
  wallpaper->info.frameDelay = frame_delay * 1000;
}

bool Decoder::decodeFrame(std::shared_ptr<Frame> frame) {
  int response = true;

  while (true) {
    response = avcodec_receive_frame(*codecContext, *frame);
    if (response == AVERROR(EAGAIN)) {
      // try again
      break;
    } else if (response == AVERROR_EOF) {
      // get eof, decode finish
      decodeFinished = true;
      LOG(INFO) << "Decode finished";
      return true;
    } else if (response < 0) {
      LOG(ERROR) << "Error while receiving frame from decoder.";
      response = false;
      break;
    } else {
      // got an frame
      response = handleFrame(frame);
    }
  }

  return response;
}

bool Decoder::decode() {
  LOG(INFO) << "Start decode";
  std::unique_ptr<Packet> packet = std::make_unique<Packet>();
  if (!(*packet)) {
    return false;
  }

  std::shared_ptr<Frame> frame = std::make_shared<Frame>();
  if (!(*frame)) {
    return false;
  }

  int response = 0;
  decodeFinished = false;
  while (av_read_frame(*formatContext, *packet) >= 0) {
    if ((*packet)->stream_index != videoStreamIndex) {
      continue;
    }

    if (avcodec_send_packet(*codecContext, *packet) < 0) {
      LOG(ERROR) << "Error while sending packet to decoder.";
      response = false;
      break;
    }

    response = decodeFrame(frame);

    if (wallpaper->type == STATIC) break;

    // To prevent the renderer from waiting indefinitely due to a lack of
    // frames because the decoder stsignals working first when the frame
    // queue is empty.
    if (signal && !frameBuffer.empty()) break;

    if (frameBuffer.size() > 20)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

    av_packet_unref(*packet);
  }

  if (avcodec_send_packet(*codecContext, nullptr) >= 0)
    response = decodeFrame(frame);

  return response;
}

Decoder::Decoder(std::atomic_bool& signal, FrameBuffer& frameBuffer)
    : signal(signal), frameBuffer(frameBuffer) {}

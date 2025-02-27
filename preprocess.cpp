#include "preprocess.h"

#include "paper.h"

namespace Wow {

bool Preprocess::decodeStatic(StaticWallpaper& wallpaper) {
  LOG(INFO) << "Decode wallpaper: " << wallpaper.name;

  // Find the info of media
  AVFormatContext* formatContext = avformat_alloc_context();
  if (avformat_open_input(&formatContext, wallpaper.name.c_str(), nullptr,
                          nullptr) != 0) {
    LOG(ERROR) << "Could not open input file: " << wallpaper.name;
    return false;
  }

  if (avformat_find_stream_info(formatContext, nullptr) < 0) {
    LOG(ERROR) << "Could not find stream information for file: "
               << wallpaper.name;
    return false;
  }

  // Init decode
  const AVCodec* codec = ::avcodec_find_decoder(AV_CODEC_ID_WEBP);
  if (codec == nullptr) {
    LOG(ERROR) << "Could not find codec.";
    return false;
  }

  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
  if (codecContext == nullptr) {
    LOG(ERROR) << "Could not allocate codec context.";
    return false;
  }

  if (::avcodec_parameters_to_context(
          codecContext, formatContext->streams[0]->codecpar) < 0) {
    LOG(ERROR) << "Could not copy codec parameters to context for file: "
               << wallpaper.name;
    return false;
  }

  if (avcodec_open2(codecContext, codec, nullptr) < 0) {
    LOG(ERROR) << "Could not open codec.";
    return false;
  }

  AVPacket* packet = av_packet_alloc();
  AVFrame* frame = av_frame_alloc();
  int response = 0;

  while (av_read_frame(formatContext, packet) >= 0) {
    response = avcodec_send_packet(codecContext, packet);
    if (response < 0) {
      LOG(ERROR) << "Error while sending packet to decoder.";
      break;
    }

    while (response >= 0) {
      response = avcodec_receive_frame(codecContext, frame);
      if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
        break;
      } else if (response < 0) {
        LOG(ERROR) << "Error while receiving frame from decoder.";
        return false;
      }

      struct SwsContext* sws_ctx = sws_getContext(
          frame->width, frame->height, (AVPixelFormat)frame->format,
          frame->width, frame->height, AV_PIX_FMT_RGB24, SWS_SINC, NULL, NULL,
          NULL);

      if (!sws_ctx) {
        LOG(ERROR) << "Could not initialize the conversion context.";
        return false;
      }

      AVFrame* rgb_frame = av_frame_alloc();
      rgb_frame->format = AV_PIX_FMT_RGB24;
      rgb_frame->width = frame->width;
      rgb_frame->height = frame->height;

      if (av_frame_get_buffer(rgb_frame, 0) < 0) {
        LOG(ERROR) << "Could not allocate buffer for converted frame.";
        return false;
      }

      sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
                rgb_frame->data, rgb_frame->linesize);

      wallpaper.size.scaledWidth = rgb_frame->width;
      wallpaper.size.scaledHeight = rgb_frame->height;

      int num_bytes = av_image_get_buffer_size(
          AV_PIX_FMT_RGB24, rgb_frame->width, rgb_frame->height, 1);
      wallpaper.buffer.resize(num_bytes);

      av_image_copy_to_buffer(wallpaper.buffer.data(), num_bytes,
                              rgb_frame->data, rgb_frame->linesize,
                              AV_PIX_FMT_RGB24, rgb_frame->width,
                              rgb_frame->height, 1);

      av_frame_free(&rgb_frame);
      sws_freeContext(sws_ctx);

      break;
    }

    av_packet_unref(packet);
  }

end:
  av_packet_free(&packet);
  av_frame_free(&frame);
  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);
  return true;
}

bool Preprocess::decodeDynamic(DynamicWallpaper& wallpaper) {
  LOG(INFO) << "Decode dynamic wallpaper: " << wallpaper.name;

  // Find the format info
  AVFormatContext* formatContext = nullptr;
  if (avformat_open_input(&formatContext, wallpaper.name.c_str(), nullptr,
                          nullptr) != 0) {
    LOG(ERROR) << "Can't open the input file: " << wallpaper.name;
    return false;
  }

  if (avformat_find_stream_info(formatContext, nullptr) < 0) {
    LOG(ERROR) << "Can't find the stream info: " << wallpaper.name;
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
    LOG(ERROR) << "Can't find the video stream: " << wallpaper.name;
    avformat_close_input(&formatContext);
    return false;
  }

  // Get the frame rate
  AVStream* videoStream = formatContext->streams[videoStreamIndex];
  AVRational frame_rate =
      av_guess_frame_rate(formatContext, videoStream, nullptr);
  // Calculate the delay time each frame
  double frame_delay = av_q2d(av_inv_q(frame_rate));
  wallpaper.frameDelay = frame_delay * 1000;

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

  // Alloc the frame and packet
  AVFrame* frame = av_frame_alloc();
  AVPacket* packet = av_packet_alloc();

  // Init SwsContext and alloc the rgb frame
  struct SwsContext* sws_ctx = nullptr;
  AVFrame* rgb_frame = av_frame_alloc();

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
          wallpaper.size.scaledWidth = frame->width;
          wallpaper.size.scaledHeight = frame->height;
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

        // Save the frame the data
        wallpaper.bufferStream.push_back(std::move(buffer));
      }
    }
    av_packet_unref(packet);
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

void Preprocess::decode(Wallpaper& paper) {
  if (paper.type == Wallpaper::STATIC) {
    StaticWallpaper& wallpaper = static_cast<StaticWallpaper&>(paper);
    if (wallpaper.buffer.empty()) {
      decodeStatic(wallpaper);
    }
  } else {
    DynamicWallpaper& wallpaper = static_cast<DynamicWallpaper&>(paper);
    if (wallpaper.bufferStream.empty()) {
      decodeDynamic(wallpaper);
    }
  }
}

bool Preprocess::scale(Wallpaper& wallpaper) {
  // TODO:
  return false;
}

}  // namespace Wow

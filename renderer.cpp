#include "renderer.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "paper.h"

namespace Wow {

Renderer::Renderer(bool &signal) : signal(signal) {
  if (initX11()) {
    sdlWindow = SDL_CreateWindowFrom((void *)(uintptr_t)xwindow);
    renderer = SDL_CreateRenderer(
        sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
  }
  if (!sdlWindow) {
    LOG(ERROR) << "SDL_CreateWindowFrom failed:" << SDL_GetError();
    return;
  }
}

bool Renderer::initX11() {
  screenWidth =
      DisplayWidth(XOpenDisplay(NULL), DefaultScreen(XOpenDisplay(NULL)));
  screenHeight =
      DisplayHeight(XOpenDisplay(NULL), DefaultScreen(XOpenDisplay(NULL)));

  display = XOpenDisplay(NULL);
  if (!display) {
    LOG(ERROR) << "Unable to open display device！";
    return false;
  }

  int screen = DefaultScreen(display);
  Window root = RootWindow(display, screen);

  XSetWindowAttributes xswa;
  xswa.override_redirect = False;
  xswa.event_mask = ExposureMask;

  xwindow = XCreateWindow(display, root, 0, 0, screenWidth, screenHeight, 0,
                          CopyFromParent, InputOutput, CopyFromParent,
                          CWOverrideRedirect | CWEventMask, &xswa);

  Atom wm_dow_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
  Atom wm_window_type_desktop =
      XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
  XChangeProperty(display, xwindow, wm_dow_type, XA_ATOM, 32, PropModeReplace,
                  (unsigned char *)&wm_window_type_desktop, 1);

  Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
  Atom wm_state_below = XInternAtom(display, "_NET_WM_STATE_BELOW", False);
  Atom wm_state_skip_taskbar =
      XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", False);
  Atom wm_state_sticky = XInternAtom(display, "_NET_WM_STATE_STICKY", False);
  Atom wm_states[] = {wm_state_below, wm_state_skip_taskbar, wm_state_sticky};
  XChangeProperty(display, xwindow, wm_state, XA_ATOM, 32, PropModeReplace,
                  (unsigned char *)wm_states, 3);

  Atom wm_desktop = XInternAtom(display, "_NET_WM_DESKTOP", False);
  long desktop = 0xFFFFFFFF;
  XChangeProperty(display, xwindow, wm_desktop, XA_CARDINAL, 32,
                  PropModeReplace, (unsigned char *)&desktop, 1);

  XMapWindow(display, xwindow);
  XFlush(display);
  return true;
}

Renderer::~Renderer() {
  if (sdlWindow) SDL_DestroyWindow(sdlWindow);
  if (renderer) SDL_DestroyRenderer(renderer);
  if (xwindow) XDestroyWindow(display, xwindow);
  if (display) XCloseDisplay(display);
}

void Renderer::render(Wallpaper &wallpaper) {
  LOG(INFO) << "Wallpaper info: name:" << wallpaper.name
            << " type:" << wallpaper.type
            << " width:" << wallpaper.size.scaledWidth
            << " height:" << wallpaper.size.scaledHeight;

  if (wallpaper.type == Wallpaper::STATIC) {
    renderStatic(static_cast<StaticWallpaper &>(wallpaper));
  } else {
    renderDynamic(static_cast<DynamicWallpaper &>(wallpaper));
  }
}

void Renderer::renderStatic(StaticWallpaper &wallpaper) {
  SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
      wallpaper.buffer.data(), wallpaper.size.scaledWidth,
      wallpaper.size.scaledHeight, 24, wallpaper.size.scaledWidth * 3,
      0x000000ff, 0x0000ff00, 0x00ff0000, 0);

  if (surface == nullptr) {
    LOG(ERROR) << "Failed to create SDL_Surface: " << SDL_GetError();
    return;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == NULL) {
    LOG(ERROR) << "Failed to create SDL_Texture: " << SDL_GetError();
  }

  SDL_FreeSurface(surface);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

  int times = 50;
  while (times--) {
    if (signal) {
      break;
    }
    SDL_Delay(1000);
  }
  SDL_DestroyTexture(texture);
}

void Renderer::renderDynamic(DynamicWallpaper &wallpaper) {
  SDL_Texture *texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
      wallpaper.size.scaledWidth, wallpaper.size.scaledHeight);
  if (texture == NULL) {
    LOG(ERROR) << "Failed to create SDL_Texture: " << SDL_GetError();
    return;
  }

  for (const auto &frame_buffer : wallpaper.bufferStream) {
    void *pixels;
    int pitch;
    if (SDL_LockTexture(texture, NULL, &pixels, &pitch) < 0) {
      LOG(ERROR) << "Can't lock the texture: " << SDL_GetError();
      continue;
    }

    // Calculate pitch
    int expected_pitch = wallpaper.size.scaledWidth * 3;
    if (pitch != expected_pitch) {
      LOG(WARNING) << "texture's pitch:" << pitch
                   << " is not in line with expectations:" << expected_pitch;
    }

    // Copy the pixels data
    uint8_t *dst = static_cast<uint8_t *>(pixels);
    const uint8_t *src = frame_buffer.data();
    for (int y = 0; y < wallpaper.size.scaledHeight; ++y) {
      memcpy(dst + y * pitch, src + y * expected_pitch, expected_pitch);
    }

    SDL_UnlockTexture(texture);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(static_cast<Uint32>(wallpaper.frameDelay));

    if (signal) {
      break;
    }
  }
  SDL_DestroyTexture(texture);
}

}  // namespace Wow

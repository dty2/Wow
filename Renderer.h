#ifndef RENDERER_H
#define RENDERER_H

#include "Common.h"
#include "WallPaper.h"

#ifdef X11
#include <SDL2/SDL.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#else
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <wlr-layer-shell-unstable-v1-client-protocol.h>
#endif

namespace Wow {

class Renderer {
#ifdef X11
  Window xwindow;
  Display* display;
  bool initX11();
  int screenWidth;
  int screenHeight;
  SDL_Window* sdlWindow;
  SDL_Renderer* renderer;
#else
  bool initWayland();
  bool initRender();
#endif

  void renderStatic(StaticWallPaperPtr wallpaper);
  void renderDynamic(DynamicWallPaperPtr wallpaper);
  bool& signal;
  int staticIntervalTime;
  int dynamicIntervalTime;

 public:
  Renderer(bool& signal);
  void render(WallPaperPtr wallPaperPtr);
  ~Renderer();
};

}  // namespace Wow

#endif  // RENDERER_H

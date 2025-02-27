#ifndef RENDERER_H
#define RENDERER_H

#include "common.h"
#include "paper.h"

extern "C" {
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#include <SDL2/SDL.h>

namespace Wow {

class Renderer {
  bool initX11();
  int screenWidth;
  int screenHeight;
  Window xwindow;
  SDL_Window* sdlWindow;
  Display* display;
  void renderStatic(StaticWallpaper& wallpaper);
  void renderDynamic(DynamicWallpaper& wallpaper);
  bool& signal;

 public:
  SDL_Renderer* renderer;
  Renderer(bool& signal);
  void render(Wallpaper& wallpaper);
  ~Renderer();
};

}  // namespace Wow

#endif  // RENDERER_H

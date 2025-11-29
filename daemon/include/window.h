#pragma once

#include <EGL/egl.h>

struct Window {
  int width = 1920;
  int height = 1080;
};

extern Window window;

bool createWindow();
void releaseWindow();
void changeSurface();

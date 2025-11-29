#include "window.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

Window window{1920, 1080};

extern EGLDisplay egl_display;
extern EGLSurface egl_surface;

bool initWayland();
bool initEgl();
void releaseEgl();
void releaseWayland();

bool createWindow() { return initWayland() && initEgl(); }

void releaseWindow() {
  releaseEgl();
  releaseWayland();
}

void changeSurface() { eglSwapBuffers(egl_display, egl_surface); }

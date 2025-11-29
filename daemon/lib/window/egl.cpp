#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-egl.h>

#include <cstddef>

#include "log.hpp"

struct Window {
  int width = 1920;
  int height = 1080;
};

extern Window window;

extern wl_surface* surface;
extern wl_display* display;
extern wl_egl_window* egl_window;
extern int configured;

EGLDisplay egl_display = EGL_NO_DISPLAY;
EGLSurface egl_surface = EGL_NO_SURFACE;
EGLConfig egl_config = NULL;
EGLContext egl_context = EGL_NO_CONTEXT;

// clang-format off
static const EGLint config_attribs[] = {
    EGL_RED_SIZE,           8,
    EGL_GREEN_SIZE,         8,
    EGL_BLUE_SIZE,          8,
    EGL_ALPHA_SIZE,         8,
    EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
    EGL_NONE,
};

static const EGLint context_attribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 3,
    EGL_NONE,
};
// clang-format on

void releaseEgl() {
  if (egl_display != EGL_NO_DISPLAY) {
    eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (egl_surface != EGL_NO_SURFACE) {
      eglDestroySurface(egl_display, egl_surface);
      egl_surface = EGL_NO_SURFACE;
    }

    if (egl_context != EGL_NO_CONTEXT)
      eglDestroyContext(egl_display, egl_context), egl_context = EGL_NO_CONTEXT;

    if (egl_window) {
      wl_egl_window_destroy(egl_window);
      egl_window = NULL;
    }

    eglTerminate(egl_display);
    egl_display = EGL_NO_DISPLAY;
    eglReleaseThread();
  }
}

bool initEgl() {
  while (!configured) {
    // Wait for compositor set the window size
    wl_display_roundtrip(display);
  }

  const std::string_view client_exts_str =
      eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);

  if (client_exts_str.empty()) {
    if (eglGetError() == EGL_BAD_DISPLAY) {
      LOG(ERROR) << "EGL_EXT_client_extensions not supported\n";
    } else {
      LOG(ERROR) << "Failed to query EGL client extensions\n";
    }
    return false;
  }

  if (client_exts_str.find("EGL_EXT_platform_base") == std::string_view::npos) {
    LOG(ERROR) << "EGL_EXT_platform_base not supported";
    return false;
  }

  if (client_exts_str.find("EGL_EXT_platform_wayland") ==
      std::string_view::npos) {
    LOG(ERROR) << "EGL_EXT_platform_wayland not supported";
    return false;
  }

  egl_display = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_EXT, display, NULL);

  if (egl_display == EGL_NO_DISPLAY) {
    LOG(ERROR) << "Failed to create EGL display";
    return false;
  }

  if (eglInitialize(egl_display, NULL, NULL) == EGL_FALSE) {
    LOG(ERROR) << "Failed to initialize EGL";
    return false;
  }

  EGLint matched = 0;
  if (!eglChooseConfig(egl_display, config_attribs, &egl_config, 1, &matched)) {
    LOG(ERROR) << "eglChooseConfig failed";
    goto error;
  }

  if (matched == 0) {
    LOG(ERROR) << "Failed to match an EGL config";
    goto error;
  }

  eglBindAPI(EGL_OPENGL_ES_API);
  egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT,
                                 context_attribs);
  if (egl_context == EGL_NO_CONTEXT) {
    LOG(ERROR) << "Failed to create EGL context";
    goto error;
  }

  egl_window = wl_egl_window_create(surface, window.width, window.height);
  if (!egl_window) {
    LOG(ERROR) << "Wayland-egl window create failed";
    goto error;
  }

  egl_surface =
      eglCreatePlatformWindowSurface(egl_display, egl_config, egl_window, NULL);
  if (egl_surface == EGL_NO_SURFACE) {
    LOG(ERROR) << "eglCreateWindowSurface failed";
    goto error;
  }

  if (!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context)) {
    LOG(ERROR) << "eglMakeCurrent failed";
    goto error;
  }

  wl_display_roundtrip(display);

  return true;

error:
  releaseEgl();
  return false;
}

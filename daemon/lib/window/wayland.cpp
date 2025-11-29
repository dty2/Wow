#include <wayland-client.h>
#include <wayland-egl.h>

#include <string_view>

#include "log.hpp"

#define namespace namespace_
extern "C" {
#include "wlr-layer-shell-client-protocol.h"
}
#undef namespace

struct Window {
  int width = 1920;
  int height = 1080;
};

extern Window window;

wl_surface* surface;
wl_display* display;
wl_egl_window* egl_window;
int configured;

static zwlr_layer_surface_v1* layer_surface;
static wl_compositor* compositor;
static wl_shm* shm;
static zwlr_layer_shell_v1* layer_shell;
static wl_output* output;

static void registryHandler(void* data, struct wl_registry* registry,
                            uint32_t id, const char* interface,
                            uint32_t version) {
  std::string_view interFace(interface);
  if (interFace == wl_compositor_interface.name) {
    compositor = (wl_compositor*)wl_registry_bind(registry, id,
                                                  &wl_compositor_interface, 4);
  } else if (interFace == wl_shm_interface.name) {
    shm = (wl_shm*)wl_registry_bind(registry, id, &wl_shm_interface, 1);
  } else if (interFace == zwlr_layer_shell_v1_interface.name) {
    layer_shell = (zwlr_layer_shell_v1*)wl_registry_bind(
        registry, id, &zwlr_layer_shell_v1_interface, 1);
  } else if (interFace == wl_output_interface.name) {
    if (!output) {
      output =
          (wl_output*)wl_registry_bind(registry, id, &wl_output_interface, 1);
    }
  }
}

static void registryRemover(void* data, struct wl_registry* registry,
                            uint32_t id) {}

static void handleLayerSurfaceConfigure(
    void* data, struct zwlr_layer_surface_v1* layer_surface, uint32_t serial,
    uint32_t w, uint32_t h) {
  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
  if (!configured || w != window.width || h != window.height) {
    window.width = w;
    window.height = h;
    configured = 1;

    if (egl_window) {
      wl_egl_window_resize(egl_window, window.width, window.height, 0, 0);
    }
    wl_surface_commit(surface);
  }
}

static void handleLayerSurfaceClosed(void* data,
                                     struct zwlr_layer_surface_v1* surface) {}

static const struct wl_registry_listener registry_listener = {
    .global = registryHandler,
    .global_remove = registryRemover,
};

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = handleLayerSurfaceConfigure,
    .closed = handleLayerSurfaceClosed,
};

bool initWayland() {
  display = wl_display_connect(NULL);

  if (!display) {
    LOG(ERROR) << "Failed to connect to Wayland display";
    return false;
  }

  // Get wayland registry table
  wl_registry* registry = wl_display_get_registry(display);

  wl_registry_add_listener(registry, &registry_listener, NULL);
  wl_display_roundtrip(display);

  // Check
  if (!compositor || !layer_shell || !output) {
    LOG(ERROR) << (stderr, "Failed to get necessary Wayland interfaces\n");
    return false;
  }

  surface = wl_compositor_create_surface(compositor);

  layer_surface = zwlr_layer_shell_v1_get_layer_surface(
      layer_shell, surface, output, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND,
      "wow_wallpaper");

  // use layer-shell create background layer surface
  zwlr_layer_surface_v1_set_anchor(layer_surface,
                                   ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);

  zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener,
                                     NULL);

  // Commit init status, wait for configure event which send by compositor
  // set window.width / window.height
  wl_surface_commit(surface);
  wl_display_roundtrip(display);

  return true;
}

void releaseWayland() {
  if (display) {
    wl_display_disconnect(display);
  }
}

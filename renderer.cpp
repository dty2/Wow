#include "renderer.h"

#include "paper.h"

namespace Wow {
#ifdef X11
bool Renderer::initX11() {
  screenWidth =
      DisplayWidth(XOpenDisplay(NULL), DefaultScreen(XOpenDisplay(NULL)));
  screenHeight =
      DisplayHeight(XOpenDisplay(NULL), DefaultScreen(XOpenDisplay(NULL)));

  display = XOpenDisplay(NULL);
  if (!display) {
    LOG(ERROR) << "Unable to open display device!";
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

void Renderer::renderStatic(StaticWallpaper &wallpaper) {
  SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
      wallpaper.buffer.data(), wallpaper.size.width, wallpaper.size.height, 24,
      wallpaper.size.width * 3, 0x000000ff, 0x0000ff00, 0x00ff0000, 0);

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
  enum { NOTEMPTY, EMPTY, FINISH } framestatus = EMPTY;

  while (framestatus != NOTEMPTY) {
    {
      std::lock_guard<std::mutex> lock(wallpaper.mtx);
      if (!wallpaper.buffer.empty()) {
        framestatus = NOTEMPTY;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  SDL_Texture *texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
      wallpaper.size.width, wallpaper.size.height);
  if (texture == NULL) {
    LOG(ERROR) << "Failed to create SDL_Texture: " << SDL_GetError();
    return;
  }

  LOG(INFO) << "Start render...";
  while (true) {
    std::vector<uint8_t> frame;
    {
      std::lock_guard<std::mutex> lock(wallpaper.mtx);
      if (!wallpaper.buffer.empty()) {
        if ([&](std::vector<uint8_t> vec1, std::vector<uint8_t> vec2) {
              if (vec1.size() >= 5 && vec2.size() >= 5 &&
                  std::equal(vec1.begin(), vec1.begin() + 5, vec2.begin())) {
                return true;
              } else {
                return false;
              }
            }(wallpaper.buffer.front(), {5, 0, 0, 0, 0, 0, 0, 0})) {
          wallpaper.buffer.pop();
          framestatus = FINISH;
        } else {
          frame = wallpaper.buffer.front();
          wallpaper.buffer.pop();
          framestatus = NOTEMPTY;
        }
      } else {
        framestatus = EMPTY;
      }
    }

    if (framestatus == FINISH) {
      break;
    } else if (framestatus == EMPTY) {
      continue;
    }

    void *pixels;
    int pitch;
    if (SDL_LockTexture(texture, NULL, &pixels, &pitch) < 0) {
      LOG(ERROR) << "Can't lock the texture: " << SDL_GetError();
      continue;
    }

    // Calculate pitch
    int expected_pitch = wallpaper.size.width * 3;
    if (pitch != expected_pitch) {
      LOG(WARNING) << "texture's pitch:" << pitch
                   << " is not in line with expectations:" << expected_pitch;
    }

    // Copy the pixels data
    uint8_t *dst = static_cast<uint8_t *>(pixels);
    const uint8_t *src = frame.data();
    for (int y = 0; y < wallpaper.size.height; ++y) {
      memcpy(dst + y * pitch, src + y * expected_pitch, expected_pitch);
    }

    SDL_UnlockTexture(texture);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(static_cast<Uint32>(wallpaper.frameDelay));

    if (signal) {
      std::lock_guard<std::mutex> lock(wallpaper.mtx);
      while (!wallpaper.buffer.empty()) {
        wallpaper.buffer.pop();
      }
      if (wallpaper.buffer.empty()) {
        LOG(INFO) << "wallpaper " << wallpaper.name << " is empty";
      }
      google::FlushLogFiles(google::INFO);
      google::FlushLogFiles(google::WARNING);
      google::FlushLogFiles(google::ERROR);
      break;
    }
  }

  SDL_DestroyTexture(texture);
}
#endif

#ifdef WAYLAND

// -----------------------------------------------
//       Create Wallpaper Window in wayland
// -----------------------------------------------

static struct {
  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;
  int width = 1920;
  int height = 1080;
  int configured;
} window = {0};

static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct zwlr_layer_shell_v1 *layer_shell = NULL;
static struct wl_output *output = NULL;

static void registryHandler(void *data, struct wl_registry *registry,
                            uint32_t id, const char *interface,
                            uint32_t version) {
  if (strcmp(interface, "wl_compositor") == 0) {
    compositor = (struct wl_compositor *)wl_registry_bind(
        registry, id, &wl_compositor_interface, 4);
  } else if (strcmp(interface, "wl_shm") == 0) {
    shm = (struct wl_shm *)wl_registry_bind(registry, id, &wl_shm_interface, 1);
  } else if (strcmp(interface, "zwlr_layer_shell_v1") == 0) {
    layer_shell = (struct zwlr_layer_shell_v1 *)wl_registry_bind(
        registry, id, &zwlr_layer_shell_v1_interface, 1);
  } else if (strcmp(interface, "wl_output") == 0) {
    if (!output) {
      output = (struct wl_output *)wl_registry_bind(registry, id,
                                                    &wl_output_interface, 1);
    }
  }
}

static void registryRemover(void *data, struct wl_registry *registry,
                            uint32_t id) {}

static void handleLayerSurfaceConfigure(void *data,
                                        struct zwlr_layer_surface_v1 *surface,
                                        uint32_t serial, uint32_t width,
                                        uint32_t height) {
  zwlr_layer_surface_v1_ack_configure(surface, serial);
  if (!window.configured || width != window.width || height != window.height) {
    window.width = width;
    window.height = height;
    window.configured = 1;
    wl_surface_commit(window.surface);
  }
}

static void handleLayerSurfaceClosed(void *data,
                                     struct zwlr_layer_surface_v1 *surface) {}

bool Renderer::initWayland() {
  display = wl_display_connect(NULL);
  if (!display) {
    LOG(ERROR) << "Failed to connect to Wayland display";
    return false;
  }

  // Get wayland registry table
  struct wl_registry *registry = wl_display_get_registry(display);
  const struct wl_registry_listener registry_listener = {
      .global = registryHandler,
      .global_remove = registryRemover,
  };
  wl_registry_add_listener(registry, &registry_listener, NULL);
  wl_display_roundtrip(display);

  // Check
  if (!compositor || !layer_shell || !output) {
    LOG(ERROR) << (stderr, "Failed to get necessary Wayland interfaces\n");
    return false;
  }

  // Create surface
  window.surface = wl_compositor_create_surface(compositor);

  // use layer-shell create background layer surface
  window.layer_surface = zwlr_layer_shell_v1_get_layer_surface(
      layer_shell, window.surface, output, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND,
      "example-wallpaper");
  zwlr_layer_surface_v1_set_anchor(window.layer_surface,
                                   ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
  zwlr_layer_surface_v1_set_keyboard_interactivity(window.layer_surface, false);

  static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
      .configure = handleLayerSurfaceConfigure,
      .closed = handleLayerSurfaceClosed,
  };
  zwlr_layer_surface_v1_add_listener(window.layer_surface,
                                     &layer_surface_listener, NULL);

  // Commit init status, wait for configure event which send by compositor
  // set window width / height
  wl_surface_commit(window.surface);
  return true;
}

// -------------------------------------
//           Create Renderer
// -------------------------------------

// OpenGL ES 2.0 shader source code
static const char *vertexShaderSource = R"(
    attribute vec2 aPos;
    attribute vec2 aTex;
    varying vec2 vTex;
    void main(){
        gl_Position = vec4(aPos, 0.0, 1.0);
        vTex = aTex;
    }
)";

static const char *fragmentShaderSource = R"(
    precision mediump float;
    varying vec2 vTex;
    uniform sampler2D uTexture;
    void main(){
        gl_FragColor = texture2D(uTexture, vTex);
    }
)";

// Compile shader code
static GLuint compileShader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    char log[512];
    glGetShaderInfoLog(shader, sizeof(log), NULL, log);
    fprintf(stderr, "Shader compile error: %s\n", log);
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

// Create shader program
static GLuint createProgram(const char *vsrc, const char *fsrc) {
  GLuint vShader = compileShader(GL_VERTEX_SHADER, vsrc);
  if (!vShader) return 0;
  GLuint fShader = compileShader(GL_FRAGMENT_SHADER, fsrc);
  if (!fShader) return 0;
  GLuint program = glCreateProgram();
  glAttachShader(program, vShader);
  glAttachShader(program, fShader);
  // Specify the attribute position
  glBindAttribLocation(program, 0, "aPos");
  glBindAttribLocation(program, 1, "aTex");
  glLinkProgram(program);
  GLint linked = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (!linked) {
    char log[512];
    glGetProgramInfoLog(program, sizeof(log), NULL, log);
    fprintf(stderr, "Program link error: %s\n", log);
    glDeleteProgram(program);
    return 0;
  }
  glDeleteShader(vShader);
  glDeleteShader(fShader);
  return program;
}

// Full-screen quadrilateral vertex data
// (two triangles form a rectangle, including position and texture coordinates)
// Chinese Comments: 全屏四边形顶点数据（两个三角形组成矩形，含位置和纹理坐标）
static const GLfloat quadVertices[] = {
    // x,    y,     u,   v
    -1.0f, -1.0f, 0.0f, 1.0f,  // left down
    1.0f,  -1.0f, 1.0f, 1.0f,  // right down
    -1.0f, 1.0f,  0.0f, 0.0f,  // left up
    1.0f,  1.0f,  1.0f, 0.0f   // right up
};

// EGL and OpenGL init variables
static EGLDisplay eglDisplay = EGL_NO_DISPLAY;
static EGLContext eglContext = EGL_NO_CONTEXT;
static EGLSurface eglSurface = EGL_NO_SURFACE;
static struct wl_egl_window *eglWindow = NULL;
static GLuint shaderProgram = 0;
static GLuint vbo = 0;

// Init EGL and OpenGL
static bool initEGL_GL() {
  // Get EGLDisplay
  eglDisplay = eglGetDisplay((EGLNativeDisplayType)display);
  if (eglDisplay == EGL_NO_DISPLAY) {
    LOG(ERROR) << "eglGetDisplay failed";
    return false;
  }
  EGLint major, minor;
  if (!eglInitialize(eglDisplay, &major, &minor)) {
    LOG(ERROR) << "eglInitialize failed";
    return false;
  }

  EGLint configAttribs[] = {EGL_SURFACE_TYPE,
                            EGL_WINDOW_BIT,
                            EGL_RENDERABLE_TYPE,
                            EGL_OPENGL_ES2_BIT,
                            EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_NONE};
  EGLConfig config;
  EGLint numConfigs;
  if (!eglChooseConfig(eglDisplay, configAttribs, &config, 1, &numConfigs) ||
      numConfigs < 1) {
    LOG(ERROR) << "eglChooseConfig failed";
    return false;
  }

  EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  eglContext =
      eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
  if (eglContext == EGL_NO_CONTEXT) {
    LOG(ERROR) << "eglCreateContext failed";
    return false;
  }
  // Create wl_egl_window
  // Use hight and width send by Wayland surface and compositor
  eglWindow = wl_egl_window_create(window.surface, window.width, window.height);
  if (!eglWindow) {
    LOG(ERROR) << "wl_egl_window_create failed";
    return false;
  }
  eglSurface = eglCreateWindowSurface(eglDisplay, config,
                                      (EGLNativeWindowType)eglWindow, NULL);
  if (eglSurface == EGL_NO_SURFACE) {
    LOG(ERROR) << "eglCreateWindowSurface failed";
    return false;
  }
  if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
    LOG(ERROR) << "eglMakeCurrent failed";
    return false;
  }

  // Set view port
  glViewport(0, 0, window.width, window.height);

  // Create shader program
  shaderProgram = createProgram(vertexShaderSource, fragmentShaderSource);
  if (!shaderProgram) return false;

  // Create a VBO to store the full screen Quad
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
               GL_STATIC_DRAW);

  return true;
}

bool Renderer::initRender() {
  while (!window.configured) {
    // Wait for compositor set the window size
    wl_display_roundtrip(display);
  }

  // Init EGL and OpenGL ES2 context
  if (!initEGL_GL()) {
    LOG(ERROR) << "EGL/GL initialization failed";
    return false;
  }

  // Run shader program
  glUseProgram(shaderProgram);
  return true;
}

void Renderer::renderDynamic(DynamicWallpaper &wallpaper) {
  enum { NOTEMPTY, EMPTY, FINISH } framestatus = EMPTY;

  while (framestatus != NOTEMPTY) {
    {
      std::lock_guard<std::mutex> lock(wallpaper.mtx);
      if (!wallpaper.buffer.empty()) {
        framestatus = NOTEMPTY;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Create OpenGL texture
  GLuint dynamicTexID;
  glGenTextures(1, &dynamicTexID);
  glBindTexture(GL_TEXTURE_2D, dynamicTexID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  LOG(INFO) << "Start render dynamic wallpaper...";
  while (true) {
    std::vector<uint8_t> frame;
    {
      std::lock_guard<std::mutex> lock(wallpaper.mtx);
      if (!wallpaper.buffer.empty()) {
        if ([&](std::vector<uint8_t> vec1, std::vector<uint8_t> vec2) {
              if (vec1.size() >= 5 && vec2.size() >= 5 &&
                  std::equal(vec1.begin(), vec1.begin() + 5, vec2.begin())) {
                return true;
              } else {
                return false;
              }
            }(wallpaper.buffer.front(), {5, 0, 0, 0, 0, 0, 0, 0})) {
          wallpaper.buffer.pop();
          framestatus = FINISH;
        } else {
          frame = wallpaper.buffer.front();
          wallpaper.buffer.pop();
          framestatus = NOTEMPTY;
        }
      } else {
        framestatus = EMPTY;
      }
    }

    if (framestatus == FINISH) {
      break;
    } else if (framestatus == EMPTY) {
      continue;
    }
    // Upload texture data using the actual video size
    int dynWidth = wallpaper.size.width, dynHeight = wallpaper.size.height;

    // Upload texture data
    glBindTexture(GL_TEXTURE_2D, dynamicTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dynWidth, dynHeight, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, frame.data());

    GLint loc = glGetUniformLocation(shaderProgram, "uTexture");
    glUniform1i(loc, 0);

    // render frame
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                          (void *)(2 * sizeof(GLfloat)));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    eglSwapBuffers(eglDisplay, eglSurface);

    // Check OpenGL error
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
      LOG(ERROR) << "OpenGL error: " << err;
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(wallpaper.frameDelay));

    if (signal) {
      std::lock_guard<std::mutex> lock(wallpaper.mtx);
      while (!wallpaper.buffer.empty()) {
        wallpaper.buffer.pop();
      }
      LOG(INFO) << "Dynamic wallpaper rendering stopped.";
      break;
    }
  }

  glDeleteTextures(1, &dynamicTexID);
}

void Renderer::renderStatic(StaticWallpaper &wallpaper) {
  GLuint texID;
  // Create texture
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int imgWidth = wallpaper.size.width;
  int imgHeight = wallpaper.size.height;

  int screenW = window.width;
  int screenH = window.height;

  float scale = std::max((float)screenW / imgWidth, (float)screenH / imgHeight);
  int scaledWidth = static_cast<int>(imgWidth * scale);
  int scaledHeight = static_cast<int>(imgHeight * scale);

  float texLeft = 0.5f - ((float)screenW / (2.0f * scaledWidth));
  float texRight = 0.5f + ((float)screenW / (2.0f * scaledWidth));
  float texBottom = 0.5f - ((float)screenH / (2.0f * scaledHeight));
  float texTop = 0.5f + ((float)screenH / (2.0f * scaledHeight));

  glBindTexture(GL_TEXTURE_2D, texID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0, GL_RGB,
               GL_UNSIGNED_BYTE, wallpaper.buffer.data());

  GLint loc = glGetUniformLocation(shaderProgram, "uTexture");
  if (loc == -1) {
    LOG(ERROR) << "Uniform 'uTexture' not found in shader program!";
    return;
  }
  glUniform1i(loc, 0);

  const GLfloat quadVertices[] = {
      -1.0f, -1.0f, texLeft, texTop,    1.0f, -1.0f, texRight, texTop,
      -1.0f, 1.0f,  texLeft, texBottom, 1.0f, 1.0f,  texRight, texBottom};

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
               GL_STATIC_DRAW);

  // Clear screen and render
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                        (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                        (void *)(2 * sizeof(GLfloat)));

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  eglSwapBuffers(eglDisplay, eglSurface);

  // Check OpenGL error
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    LOG(ERROR) << "3OpenGL error: " << err
               << " Render wallpaper :" << wallpaper.name;
  }

  int times = 60;
  while (times--) {
    if (signal) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

#endif

void Renderer::render(Wallpaper &wallpaper) {
  LOG(INFO) << "Wallpaper info: name:" << wallpaper.name
            << " type:" << wallpaper.type;

  if (wallpaper.type == Wallpaper::STATIC) {
    renderStatic(static_cast<StaticWallpaper &>(wallpaper));
  } else {
    renderDynamic(static_cast<DynamicWallpaper &>(wallpaper));
  }
}

Renderer::Renderer(bool &signal) : signal(signal) {
#ifdef WAYLAND
  initWayland();
  initRender();
#else
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
#endif
}

Renderer::~Renderer() {
#ifdef X11
  if (sdlWindow) SDL_DestroyWindow(sdlWindow);
  if (renderer) SDL_DestroyRenderer(renderer);
  if (xwindow) XDestroyWindow(display, xwindow);
  if (display) XCloseDisplay(display);
#else
  wl_display_disconnect(display);
#endif
}

}  // namespace Wow

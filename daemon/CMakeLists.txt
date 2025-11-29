pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)
pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GLES2 REQUIRED glesv2)
pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)

find_package(tomlplusplus)

add_executable(
    wow-daemon
    main.cpp
    lib/core.cpp
    lib/player.cpp
    # resource
    lib/resource/wallpaper.cpp
    lib/resource/resourcemanager.cpp
    # window
    lib/window/window.cpp
    lib/window/egl.cpp
    lib/window/wayland.cpp
    # decoder
    lib/decode/decoder.cpp
    lib/decode/ffmpeg.cpp
    # renderer
    lib/render/Shader.cpp
    lib/render/IndexBuffer.cpp
    lib/render/VertexArray.cpp
    lib/render/VertexBuffer.cpp
    lib/render/VertexBufferLayout.cpp
    lib/render/error.cpp
    lib/render/renderer.cpp
    lib/render/Texture.cpp
)

target_include_directories(
    wow-daemon
    PUBLIC
        ${BASIC_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${GENERATED_HEADER_DIR}
        ${WAYLAND_CLIENT_INCLUDE_DIRS}
        ${EGL_INCLUDE_DIRS}
        ${GLES2_INCLUDE_DIRS}
        ${FFMPEG_INCLUDE_DIRS}
        tomlplusplus/include
)

target_link_libraries(
    wow-daemon
    PUBLIC
        Protocols
        basic
        ${WAYLAND_CLIENT_LIBRARIES}
        ${WAYLAND_CLIENT_LDFLAGS_OTHER}
        ${EGL_LIBRARIES}
        ${GLES2_LIBRARIES}
        ${WAYLAND_EGL_LIBRARIES}
        ${FFMPEG_LIBRARIES}
)

set_target_properties(
    wow-daemon
    PROPERTIES BUILD_RPATH "${CMAKE_BINARY_DIR}/lib"
)

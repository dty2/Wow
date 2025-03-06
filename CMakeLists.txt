cmake_minimum_required(VERSION 3.12)

project(Wow VERSION 1.0)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glog REQUIRED)
find_package(PkgConfig)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)

if(WAYLAND)

    option(WAYLAND "Build with Wayland support" ON)
    # pkg_check_modules(Wayland REQUIRED wayland-client wayland-protocols)
    pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
    pkg_check_modules(WLR_PROTOCOLS REQUIRED wayland-protocols)
    pkg_check_modules(EGL REQUIRED egl)
    pkg_check_modules(GLES2 REQUIRED glesv2)
    pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl)
    add_definitions(-DWAYLAND)

else()

    option(X11 "Build with X11 support" ON)
    find_package(X11 REQUIRED)
    add_definitions(-DX11)

endif()

add_executable(Wow
    main.cpp
    engine.cpp
    renderer.cpp
    dashboard.cpp
    paper.cpp
    communication.cpp
    generate/wlr-layer-shell-unstable-v1-client-protocol.c
    generate/xdg-shell-protocol.c
)

if(WAYLAND)
    include_directories(
        # ${Wayland_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/generate
        ${WAYLAND_CLIENT_INCLUDE_DIRS}
        ${WLR_PROTOCOLS_INCLUDE_DIRS}
        ${EGL_INCLUDE_DIRS}
        ${GLES2_INCLUDE_DIRS}
    )
    target_link_libraries(Wow PUBLIC
        # ${Wayland_LIBRARIES}
        ${WAYLAND_CLIENT_LIBRARIES}
        ${WAYLAND_CLIENT_LDFLAGS_OTHER}
        ${WLR_PROTOCOLS_LIBRARIES}
        ${EGL_LIBRARIES}
        ${GLES2_LIBRARIES}
        ${WAYLAND_EGL_LIBRARIES}
    )

else()
    include_directories(
        ${X11_INCLUDE_DIR}
        ${SDL2_INCLUDE_DIRS}
    )
    target_link_libraries(Wow PUBLIC
        ${X11_LIBRARIES}
        ${SDL2_LIBRARIES}
    )

endif()

target_include_directories(Wow PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(Wow PUBLIC
    ${FFMPEG_LIBRARIES}
    glog::glog
)

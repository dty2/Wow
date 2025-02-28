cmake_minimum_required(VERSION 3.12)

project(Wow VERSION 1.0)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(X11 REQUIRED)
find_package(glog REQUIRED)

find_package(PkgConfig)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

add_executable(Wow
    main.cpp
    engine.cpp
    renderer.cpp
    dashboard.cpp
    paper.cpp
    communication.cpp
)

target_include_directories(Wow PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FFMPEG_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(Wow PUBLIC
    ${FFMPEG_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${X11_LIBRARIES}
    glog::glog
)

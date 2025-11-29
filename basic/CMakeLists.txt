configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.inc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.inc
    @ONLY
)

find_package(nlohmann_json REQUIRED)

set(BASIC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# used by wow-daemon and wow-cli, so make it as dynamic
add_library(basic SHARED ipc.cpp common.cpp)

target_include_directories(
    basic
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

cmake_minimum_required(VERSION 3.20)

project(Wow VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# not use exception and rtti
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options("-fno-exceptions")
    add_compile_options("-fno-rtti")
endif()

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")

find_package(PkgConfig)

include(AddProtocols)
include(GNUInstallDirs)

add_subdirectory(basic)
add_subdirectory(daemon)
add_subdirectory(client)

option(ENABLE_TEST "Enable building regression tests" ON)

if(ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

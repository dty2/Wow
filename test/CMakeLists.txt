# Only find python3 interpreter
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Get all test file
file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cases/*.test")

set(TEST_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(TEST_WORK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/workdir)
set(TEST_CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/config)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/path.py.in
    ${CMAKE_BINARY_DIR}/test/path.py
    @ONLY
)

foreach(test_file ${TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_test(
        NAME ${test_name}
        COMMAND
            ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/runtest.py
            ${test_file}
    )
endforeach()

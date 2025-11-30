add_executable(wow-cli main.cpp)

target_include_directories(
    wow-cli
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${BASIC_INCLUDE_DIR}
)

target_link_libraries(wow-cli PUBLIC basic)

set_target_properties(wow-cli PROPERTIES BUILD_RPATH "${CMAKE_BINARY_DIR}/lib")

install(TARGETS wow-cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# protocols
pkg_check_modules(WL_P REQUIRED wayland-protocols)
pkg_check_modules(WLR_P REQUIRED wlr-protocols)

pkg_get_variable(WL_P_PKGDATADIR wayland-protocols pkgdatadir)
pkg_get_variable(WLR_P_PKGDATADIR wlr-protocols pkgdatadir)

find_program(SCANNER NAMES wayland-scanner REQUIRED)

set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
set(GENERATED_HEADER_DIR "${CMAKE_BINARY_DIR}/generated/include")
set(GENERATED_SOURCE_DIR "${CMAKE_BINARY_DIR}/generated/src")

file(MAKE_DIRECTORY "${GENERATED_HEADER_DIR}")
file(MAKE_DIRECTORY "${GENERATED_SOURCE_DIR}")

set(WLP ${WL_P_PKGDATADIR}/stable/xdg-shell/xdg-shell.xml)
set(WLRP ${WLR_P_PKGDATADIR}/unstable/wlr-layer-shell-unstable-v1.xml)

set(WLP_H "${GENERATED_HEADER_DIR}/xdg-shell-client-protocol.h")
set(WLP_C "${GENERATED_SOURCE_DIR}/xdg-shell-protocol.c")

set(WLRP_H "${GENERATED_HEADER_DIR}/wlr-layer-shell-client-protocol.h")
set(WLRP_C "${GENERATED_SOURCE_DIR}/wlr-layer-shell-protocol.c")

add_custom_command(
    OUTPUT ${WLP_H} ${WLP_C}
    COMMAND ${SCANNER} client-header ${WLP} ${WLP_H}
    COMMAND ${SCANNER} private-code ${WLP} ${WLP_C}
    DEPENDS ${WLP}
)

add_custom_command(
    OUTPUT ${WLRP_H} ${WLRP_C}
    COMMAND ${SCANNER} client-header ${WLRP} ${WLRP_H}
    COMMAND ${SCANNER} private-code ${WLRP} ${WLRP_C}
    DEPENDS ${WLRP}
)

# only used by wow-daemon, so make it as static
add_library(Protocols STATIC ${WLP_C} ${WLRP_C})

target_include_directories(Protocols PUBLIC ${GENERATED_HEADER_DIR})

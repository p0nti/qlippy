include(WaylandScanner)

add_library(qlippy_platform STATIC
    WaylandConnection.cpp
    WaylandRegistry.cpp
    WaylandClipboardMonitor.cpp
    ClipboardWriter.cpp
)

set(PROTO_DIR "${CMAKE_SOURCE_DIR}/protocols")
wayland_generate_protocol(qlippy_platform
    "${PROTO_DIR}/ext-data-control-v1.xml"
    "ext-data-control-v1")
wayland_generate_protocol(qlippy_platform
    "${PROTO_DIR}/wlr-data-control-unstable-v1.xml"
    "wlr-data-control-unstable-v1")

target_include_directories(qlippy_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(qlippy_platform PUBLIC
    Qt6::Core
    Qt6::Gui
    ${WAYLAND_CLIENT_LIBRARIES}
    qlippy_common
    qlippy_storage
)

target_include_directories(qlippy_platform PUBLIC
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
)

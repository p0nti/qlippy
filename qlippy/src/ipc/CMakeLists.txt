add_library(qlippy_ipc STATIC)

target_sources(qlippy_ipc PRIVATE
    IpcMessage.cpp
    JsonCodec.cpp
    IpcServer.cpp
    IpcClient.cpp
)

target_include_directories(qlippy_ipc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(qlippy_ipc PUBLIC
    Qt6::Core
    Qt6::Network
    qlippy_common
)

add_library(qlippy_common STATIC
    Logger.cpp
)

target_include_directories(qlippy_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(qlippy_common PUBLIC
    Qt6::Core
)

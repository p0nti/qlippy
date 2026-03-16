add_library(qlippy_storage STATIC)

target_sources(qlippy_storage PRIVATE
    Storage.cpp
    HistoryRepository.cpp
    Settings.cpp
)

target_include_directories(qlippy_storage PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(qlippy_storage PUBLIC
    Qt6::Core
    Qt6::Sql
    qlippy_common
)

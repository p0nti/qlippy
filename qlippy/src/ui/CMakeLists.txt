add_library(qlippy_ui STATIC
    ClipboardModel.cpp
    PopupController.cpp
    SettingsModel.cpp
)

target_include_directories(qlippy_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(qlippy_ui PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    qlippy_common
    qlippy_storage
)

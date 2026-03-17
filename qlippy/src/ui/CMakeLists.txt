# qt_add_qml_module requires relative paths or QT_RESOURCE_ALIAS for absolute paths
foreach(_qml ClipboardWindow.qml OptionsSlider.qml ClipItemDelegate.qml ClipCompactView.qml SearchBox.qml Preview.qml)
    set_source_files_properties(
        ${CMAKE_SOURCE_DIR}/qml/${_qml}
        PROPERTIES QT_RESOURCE_ALIAS ${_qml}
    )
endforeach()

qt_add_qml_module(qlippy_ui
    URI Qlippy
    VERSION 1.0
    PLUGIN_TARGET qlippyplugin
    STATIC
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/ui/Qlippy
    QML_FILES
        ${CMAKE_SOURCE_DIR}/qml/ClipboardWindow.qml
        ${CMAKE_SOURCE_DIR}/qml/OptionsSlider.qml
        ${CMAKE_SOURCE_DIR}/qml/ClipItemDelegate.qml
        ${CMAKE_SOURCE_DIR}/qml/ClipCompactView.qml
        ${CMAKE_SOURCE_DIR}/qml/SearchBox.qml
        ${CMAKE_SOURCE_DIR}/qml/Preview.qml
    SOURCES
        ClipboardModel.cpp ClipboardModel.h
        SettingsModel.cpp SettingsModel.h
        PopupController.cpp PopupController.h
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

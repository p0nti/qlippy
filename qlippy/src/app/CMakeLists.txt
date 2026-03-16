qt_add_executable(qlippy
    main.cpp
    DaemonApp.cpp
    ${CMAKE_SOURCE_DIR}/qml/qlippy.qrc
)

target_include_directories(qlippy PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(qlippy PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Sql
    qlippy_common
    qlippy_ipc
    qlippy_storage
    qlippy_platform
    qlippy_ui
)

include(GNUInstallDirs)
install(TARGETS qlippy
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES ${CMAKE_SOURCE_DIR}/docs/qlippy.service
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/systemd/user
)

add_executable(qlippy_cli_command_tests
    CliCommandTests.cpp
)

target_link_libraries(qlippy_cli_command_tests PRIVATE
    Qt6::Test
    qlippy_storage
    qlippy_ipc
)

target_compile_definitions(qlippy_cli_command_tests PRIVATE
    QLIPPY_TEST_BIN_PATH="$<TARGET_FILE:qlippy>"
)

add_dependencies(qlippy_cli_command_tests qlippy)

add_test(NAME qlippy_cli_command_tests COMMAND qlippy_cli_command_tests)

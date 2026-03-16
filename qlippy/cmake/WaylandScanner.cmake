find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)

# wayland_generate_protocol(<target> <xml_file> <basename>)
# Generates <basename>-client-protocol.h and <basename>-protocol.c from <xml_file>
# and adds them as sources of <target>.
function(wayland_generate_protocol target xml_file basename)
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/protocols")

    set(header "${CMAKE_BINARY_DIR}/protocols/${basename}-client-protocol.h")
    set(source "${CMAKE_BINARY_DIR}/protocols/${basename}-protocol.c")

    add_custom_command(
        OUTPUT  "${header}"
        COMMAND "${WAYLAND_SCANNER}" client-header "${xml_file}" "${header}"
        DEPENDS "${xml_file}"
        VERBATIM
    )

    add_custom_command(
        OUTPUT  "${source}"
        COMMAND "${WAYLAND_SCANNER}" private-code "${xml_file}" "${source}"
        DEPENDS "${xml_file}"
        VERBATIM
    )

    # Generated file is C, not C++ — tell the compiler
    set_source_files_properties("${source}" PROPERTIES LANGUAGE C)

    target_sources(${target} PRIVATE "${source}" "${header}")
    target_include_directories(${target} PUBLIC "${CMAKE_BINARY_DIR}/protocols")
endfunction()

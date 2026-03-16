enable_testing()
find_package(Qt6 OPTIONAL_COMPONENTS Test)

if(Qt6Test_FOUND)
    # add_subdirectory(ipc)
    # add_subdirectory(storage)
endif()

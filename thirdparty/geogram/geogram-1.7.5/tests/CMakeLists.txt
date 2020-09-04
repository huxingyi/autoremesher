# Copy the RobotFramework wrapper scripts to the build directory

set(test_scripts testenv runpybot runride)

foreach(script ${test_scripts})
    configure_file_with_native_paths(
        ${PROJECT_SOURCE_DIR}/tools/${script}.${SHELL_SUFFIX}.in
        ${PROJECT_BINARY_DIR}/tests/${script}.${SHELL_SUFFIX}
        AUTODETECT
        OPTIONS @ONLY
    )
endforeach()

# Copy the valgrind suppression file to the build directory

if(UNIX)
    configure_file(
        ${PROJECT_SOURCE_DIR}/tools/valgrind.supp
        ${PROJECT_BINARY_DIR}/tests/valgrind.supp
        COPYONLY
    )
endif()

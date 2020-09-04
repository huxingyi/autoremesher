#-------------------------------------------------------------------
# Flags for compiling with Android NDK
#-------------------------------------------------------------------

include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Linux.cmake)

# Set the Android compilers
set(CMAKE_CXX_COMPILER aarch64-linux-android-clang++)
set(CMAKE_C_COMPILER aarch64-linux-android-clang)

set(VORPALINE_ARCH_64 true)

# No graphics (yet) for Android
set(GEOGRAM_WITH_GRAPHICS FALSE)

# Warning flags
set(NORMAL_WARNINGS -Wall -Wextra)
set(FULL_WARNINGS
    ${NORMAL_WARNINGS}
    -pedantic
    -Wno-long-long
    # Detect conversion problems (lot of warnings)
    -Wconversion
    -Wsign-conversion
    -Wdouble-promotion
)

# Compile with full warnings by default
add_definitions(${FULL_WARNINGS})

# Warn about missing virtual destructor (C++ only)
add_flags(CMAKE_CXX_FLAGS -Wnon-virtual-dtor)

# I do not know where this -Wno-maybe-uninitialized comes from
# (but clang does not understand it), silence the warning for 
# now...
add_flags(CMAKE_CXX_FLAGS -Wno-unknown-warning-option)
add_flags(CMAKE_C_FLAGS -Wno-unknown-warning-option)

# Activate c++ 2011
add_flags(CMAKE_CXX_FLAGS -std=c++11)

# Add static and dynamic bounds checks (optimization required)
# add_flags(CMAKE_CXX_FLAGS_RELEASE -D_FORTIFY_SOURCE=2)
# add_flags(CMAKE_C_FLAGS_RELEASE -D_FORTIFY_SOURCE=2)

# Android needs this flags, as well as -pie in final linking
set(ARCH_FLAGS -ffp-contract=off -fPIE -fPIC)
add_flags(CMAKE_CXX_FLAGS ${ARCH_FLAGS})
add_flags(CMAKE_C_FLAGS ${ARCH_FLAGS})
add_flags(CMAKE_EXE_LINKER_FLAGS ${ARCH_FLAGS} -pie)

# Additional debug flags
# deactivated for now: I added bound checking in VOR::vector<>.
#add_flags(CMAKE_CXX_FLAGS_DEBUG -D_GLIBCXX_DEBUG)

# Compile and link with pthreads
add_flags(CMAKE_CXX_FLAGS -pthread)
add_flags(CMAKE_C_FLAGS -pthread)

# Profiler compilation flags
if(VORPALINE_WITH_GPROF)
    message(STATUS "Building for code profiling")
    add_flags(CMAKE_CXX_FLAGS -pg -DPROFILER)
    add_flags(CMAKE_C_FLAGS -pg -DPROFILER)
endif()


# Code coverage compilation flags
if(VORPALINE_WITH_GCOV)
    message(STATUS "Building for coverage analysis")
    add_flags(CMAKE_CXX_FLAGS --coverage)
    add_flags(CMAKE_C_FLAGS --coverage)
endif()


# Reset the warning level for third parties
function(vor_reset_warning_level)
    remove_definitions(${FULL_WARNINGS})
    add_definitions(${NORMAL_WARNINGS})
endfunction()

macro(vor_add_executable)

    if(NOT VORPALINE_BUILD_DYNAMIC)
        # Create a statically linked executable
        # Link with static libraries
        # ... does not work with NDK 10.d
        #   (causes errors / multiply linked symbols)
#      add_flags(CMAKE_CXX_FLAGS -static-libstdc++ -static-libgcc -static)
#      add_flags(CMAKE_C_FLAGS -static-libgcc -static)
    endif()

    add_executable(${ARGN})
endmacro()


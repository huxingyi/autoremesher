#-------------------------------------------------------------------
# Flags for compiling with Mingw
#-------------------------------------------------------------------

# Shell script extension
set(SHELL_SUFFIX "sh")

# Define the environment for cross compiling to Windows
SET(CMAKE_SYSTEM_NAME    Windows) # Target system name
SET(CMAKE_SYSTEM_VERSION 1)

SET(CMAKE_AR    "x86_64-w64-mingw32-ar")
SET(CMAKE_RC_COMPILER    "x86_64-w64-mingw32-windres")
SET(CMAKE_RANLIB         "x86_64-w64-mingw32-ranlib")

# Configure the behaviour of the find commands
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(VORPALINE_ARCH_64 true)

# Link with the loader library
list(APPEND SYSLIBS dl)

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

# define the windows version
add_flags(CMAKE_CXX_FLAGS -D_WIN32_WINNT=0x601 -DMINGW_HAS_SECURE_API -Wno-unknown-pragmas)
# Activate c++ 2011
add_flags(CMAKE_CXX_FLAGS -std=c++11)

# Additional debug flags
# deactivated for now: I added bound checking in VOR::vector<>.
#add_flags(CMAKE_CXX_FLAGS_DEBUG -D_GLIBCXX_DEBUG)

# Compile and link with pthreads
add_flags(CMAKE_CXX_FLAGS -pthread)
add_flags(CMAKE_C_FLAGS -pthread)

# Reset the warning level for third parties
function(vor_reset_warning_level)
    remove_definitions(${FULL_WARNINGS})
    add_definitions(${NORMAL_WARNINGS})
endfunction()

if ( APPLE )
    string ( REPLACE "-Wl,-search_paths_first" "" CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS} )
    string ( REPLACE "-Wl,-search_paths_first" "" CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} )
endif ()

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


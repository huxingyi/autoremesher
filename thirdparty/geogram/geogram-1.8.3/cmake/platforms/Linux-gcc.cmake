#-------------------------------------------------------------------
# Flags common to all Linux based platforms with GNU compiler
#-------------------------------------------------------------------

include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Linux.cmake)

# Warning flags
set(NORMAL_WARNINGS -Wall -Wextra)
set(FULL_WARNINGS
    ${NORMAL_WARNINGS}
    -pedantic
    -Wno-long-long
    -Wconversion
)

# Determine gcc version and activate additional warnings available in latest versions
execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)

if (GCC_VERSION VERSION_GREATER 4.3 OR GCC_VERSION VERSION_EQUAL 4.3)
    set(FULL_WARNINGS ${FULL_WARNINGS} -Wsign-conversion)
endif()

if (GCC_VERSION VERSION_GREATER 4.6 OR GCC_VERSION VERSION_EQUAL 4.6)
    set(FULL_WARNINGS ${FULL_WARNINGS} -Wdouble-promotion)
endif()


# Compile with full warnings by default
add_definitions(${FULL_WARNINGS})

# Warn about missing virtual destructor (C++ only)
add_flags(CMAKE_CXX_FLAGS -Wnon-virtual-dtor)

# Add static and dynamic bounds checks (optimization required)
if (GCC_VERSION VERSION_GREATER 4.0)
   add_flags(CMAKE_CXX_FLAGS_RELEASE -D_FORTIFY_SOURCE=2)
   add_flags(CMAKE_C_FLAGS_RELEASE -D_FORTIFY_SOURCE=2)
endif()

# Enable setting FPU rounding mode (needed by FPG) and 
# disable automatic generation of FMAs (would break exact
# predicates)
add_flags(CMAKE_CXX_FLAGS -frounding-math -ffp-contract=off)
add_flags(CMAKE_C_FLAGS -frounding-math -ffp-contract=off)

# Activate AVX2 instruction set
#add_flags(CMAKE_CXX_FLAGS -mavx2)
#add_flags(CMAKE_C_FLAGS -mavx2)

# Activate c++ 2011
add_flags(CMAKE_CXX_FLAGS -std=c++11)

# Enable glibc parallel mode
#add_flags(CMAKE_CXX_FLAGS -D_GLIBCXX_PARALLEL)

# Generate debug information even in release mode
#add_flags(CMAKE_CXX_FLAGS_RELEASE -g)
#add_flags(CMAKE_C_FLAGS_RELEASE -g)

# Additional debug flags
# deactivated for now: I added bound checking in VOR::vector<>.
#add_flags(CMAKE_CXX_FLAGS_DEBUG -D_GLIBCXX_DEBUG)


# Compile and link with OpenMP
if (GCC_VERSION VERSION_GREATER 4.0)
    add_flags(CMAKE_CXX_FLAGS -fopenmp)
    add_flags(CMAKE_C_FLAGS -fopenmp)
endif()

# Alaways generate position independant code
# (to allow linking geogram/vorlalib with DLLs)
add_flags(CMAKE_CXX_FLAGS -fPIC)
add_flags(CMAKE_C_FLAGS -fPIC)

# Hide symbols that are not explicitly exported
add_flags(CMAKE_CXX_FLAGS -fvisibility=hidden)
add_flags(CMAKE_C_FLAGS -fvisibility=hidden)

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


# Compilation flags for Google's AddressSanitizer
# These flags can only be specified for dynamic builds
if(VORPALINE_WITH_ASAN)
    if(VORPALINE_BUILD_DYNAMIC)
        message(STATUS "Building with AddressSanitizer (debug only)")
        add_flags(CMAKE_CXX_FLAGS_DEBUG -fsanitize=address -fno-omit-frame-pointer)
        add_flags(CMAKE_C_FLAGS_DEBUG -fsanitize=address -fno-omit-frame-pointer)
    else()
        message(WARNING "AddressSanitizer can be used with dynamic builds only")
        set(VORPALINE_WITH_ASAN false)
    endif()
endif()
if(NOT VORPALINE_WITH_ASAN)
    # Use native GCC stack smash Protection and buffer overflow detection (debug only)
    add_flags(CMAKE_CXX_FLAGS_DEBUG -fstack-protector-all)
    add_flags(CMAKE_C_FLAGS_DEBUG -fstack-protector-all)
endif()


# Compilation flags for Google's ThreadSanitizer
# Does not work for the moment: cannot figure out how to link with library libtsan
if(VORPALINE_WITH_TSAN)
    message(STATUS "Building with ThreadSanitizer (debug only)")
    message(FATAL_ERROR "ThreadSanitizer is not available: cannot figure out how to link with library libtsan")
    add_flags(CMAKE_CXX_FLAGS_DEBUG -fsanitize=thread)
    add_flags(CMAKE_C_FLAGS_DEBUG -fsanitize=thread)
    if(NOT VORPALINE_BUILD_DYNAMIC)
        add_flags(CMAKE_EXE_LINKER_FLAGS -static-libtsan)
    endif()
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
        add_flags(CMAKE_CXX_FLAGS -static-libstdc++ -static-libgcc -static)
        add_flags(CMAKE_C_FLAGS -static-libgcc -static)
    endif()

    add_executable(${ARGN})

    if(NOT VORPALINE_BUILD_DYNAMIC AND DEFINED VORPALINE_WITH_DDT)
        # Static builds running with Allinea's DDT must be linked with a
        # special malloc library which replaces the malloc primitives of
        # the Glibc (We must allow multiple definitions)
        add_flags(CMAKE_EXE_LINKER_FLAGS -Wl,--allow-multiple-definition)

        if(VORPALINE_ARCH_64)
            link_directories(${VORPALINE_WITH_DDT}/lib/64)
        else()
            link_directories(${VORPALINE_WITH_DDT}/lib/32)
        endif()
        target_link_libraries(${ARGV0} dmallocthcxx)
    endif()

    if(UNIX)
        target_link_libraries(${ARGV0} m pthread)
    endif()
endmacro()


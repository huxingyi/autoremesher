#-------------------------------------------------------------------
# Flags common to all Linux based platforms with Intel compiler
#-------------------------------------------------------------------

include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Linux.cmake)

# Set the Intel compilers
set(CMAKE_CXX_COMPILER "/opt/intel/bin/icpc")
set(CMAKE_C_COMPILER "/opt/intel/bin/icc")
set(CMAKE_Fortran_COMPILER "/opt/intel/bin/ifort")

# Warning flags
add_definitions(
    -Wall
    -Wno-deprecated
    -Wunused
    -diag-disable 1881
    # Disable annoying remarks for Intel C++ compiler
    # remark #304: access control not specified ("public" by default)
    -wd,383,981,304
)

# Set the C standard
add_flags(CMAKE_C_FLAGS -std=c99)

# Parallelism: using openmp and enabling the "restrict" keyword.
add_flags(CMAKE_CXX_FLAGS -qopenmp -restrict)
add_flags(CMAKE_C_FLAGS -qopenmp -restrict)

# Link flags to force link of shared libs to resolve all the symbols.
add_flags(CMAKE_EXE_LINKER_FLAGS -z defs)

# icc options related with FPU:
# geogram needs strict IEEEE floating point (exact predicates depend
# on it)
add_flags(CMAKE_CXX_FLAGS -mieee-fp)
add_flags(CMAKE_C_FLAGS -mieee-fp)

# debugging symbols: uncomment the following two lines to generate them
# even in release mode (for debugging for instance).
#add_flags(CMAKE_CXX_FLAGS -g)
#add_flags(CMAKE_C_FLAGS -g)

# Reset the warning level for third parties
function(vor_reset_warning_level)
endfunction()

macro(vor_add_executable)
    if(NOT VORPALINE_BUILD_DYNAMIC)
        # Create a statically linked executable
        # Link with static libraries: temporarily deactivated (causes some errors in link phase)
        #add_flags(CMAKE_CXX_FLAGS -static -static-intel -static-libgcc)
        #add_flags(CMAKE_C_FLAGS -static -static-intel -static-libgcc)
    endif()

    add_executable(${ARGN})
endmacro()


#-------------------------------------------------------------------
# Flags for intel's "Many Integrated Cores' platform (Xeon Phi)
#-------------------------------------------------------------------

include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Linux.cmake)

# Set the Intel compilers
set(CMAKE_CXX_COMPILER icpc)
set(CMAKE_C_COMPILER icc)
set(CMAKE_Fortran_COMPILER ifort)

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

# Add Intel system includes
add_definitions(
    -isystem $ENV{INTEL}/include
    -isystem $ENV{INTEL}/include/intel64
    -isystem $ENV{INTEL}/mkl/include
    -isystem $ENV{INTEL}/ipp/include
)

# Compile and link with OpenMP
add_flags(CMAKE_CXX_FLAGS -openmp -restrict)
add_flags(CMAKE_C_FLAGS -openmp -restrict)

# Link flags to force link of shared libs to resolve all the symbols
add_flags(CMAKE_EXE_LINKER_FLAGS -z defs)

# Flags for the Release build type
# We add full report for vectorization
# (of crucial importance to get performance on MIC)
add_flags(CMAKE_CXX_FLAGS_RELEASE -ip -vec-report6)
add_flags(CMAKE_C_FLAGS_RELEASE -ip -vec-report6)

# Reset the warning level for third parties
function(vor_reset_warning_level)
endfunction()

# Create a statically linked executable
# (under MIC we create a standard shared libs executable, 
#  since static libraries are not installed)
macro(vor_add_executable)
    add_executable(${ARGN})
endmacro()


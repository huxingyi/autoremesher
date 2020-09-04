#-------------------------------------------------------------------
# Flags for Emscripten (javascript target)
#-------------------------------------------------------------------

# Shell script extension
set(SHELL_SUFFIX "sh")

find_path(EMSCRIPTEN_DIR
      emcc
      HINTS
        ENV EMSCRIPTEN
      PATHS
        "C:/Program Files/emscripten"
         /usr/lib/emscripten
)

set(CMAKE_C_COMPILER "emcc")
set(CMAKE_CXX_COMPILER "em++")
set(CMAKE_AR "emar")
set(CMAKE_RANLIB "emranlib")
set(CMAKE_LINKER "emld")
set(CMAKE_SKIP_RPATH TRUE)

include(${EMSCRIPTEN_DIR}/cmake/Modules/Platform/Emscripten.cmake)

set(GEOGRAM_WITH_EMSCRIPTEN TRUE)

# Warning flags
set(NORMAL_WARNINGS -Wall -Wextra)

set(FULL_WARNINGS
  -Weverything
    -Wno-disabled-macro-expansion # else we got a warning each time cout is used
    -Wno-padded # Disable generating a message each time padding is used
    -Wno-float-equal # Sometimes we compare floats (against 0.0 or 1.0 mainly)
    -Wno-global-constructors
    -Wno-exit-time-destructors
    -Wno-old-style-cast # Yes, old-style cast is sometime more legible...
    -Wno-format-nonliteral # Todo: use Laurent Alonso's trick
)

# Activate c++ 2011
add_flags(CMAKE_CXX_FLAGS -std=c++11 -Wno-c++98-compat -Wno-gnu-zero-variadic-macro-arguments)

# Compile with full warnings by default
add_definitions(${FULL_WARNINGS})

# Run the static analyzer
if(VORPALINE_WITH_CLANGSA)
    add_definitions(--analyze)
endif()

# Emscripten optimization flags:
# see:
#https://kripken.github.io/emscripten-site/docs/optimizing/Optimizing-Code.html
# Note: they are added to CMAKE CXX and C flags later on, because the
# way add_flags() works may remove the second "-s" argument.
# Note: TOTAL_MEMORY needs to be a multiple of 16M
set(EM_COMMON_FLAGS
  -s WASM=0
  -s USE_GLFW=3
# -s USE_WEBGL2=1 -DGEO_WEBGL2
  -s TOTAL_MEMORY=268435456
  -s EXPORTED_FUNCTIONS='["_main","_file_system_changed_callback"]'
  -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall"]'
  -s FORCE_FILESYSTEM=1
#  For now, multithreading is deactivated, because it seems that
#  browser support is not there yet !!
#  -s USE_PTHREADS=1
#  -s PTHREAD_POOL_SIZE=4
#  -s PTHREAD_HINT_NUM_CORES=1
#  -s ASSERTIONS=1
#  -s DEMANGLE_SUPPORT=1
)
set(EM_FLAGS_RELEASE -O3  ${EM_COMMON_FLAGS})
set(EM_FLAGS_DEBUG -O2 -s ASSERTIONS=2 -s SAFE_HEAP=1 -g ${EM_COMMON_FLAGS})

# Profiler compilation flags
if(VORPALINE_WITH_GPROF)
    message(FATAL_ERROR "Profiling is not (yet) available with Emscripten")
endif()

# Code coverage compilation flags
if(VORPALINE_WITH_GCOV)
    message(FATAL_ERROR "Coverage analysis not supported with Emscripten")
endif()

# Compilation flags for Google's AddressSanitizer
# These flags can only be specified for dynamic builds
if(VORPALINE_WITH_ASAN)
    message(FATAL_ERROR "Address sanitizer not supported with Emscripten")
endif()
  
if(NOT VORPALINE_WITH_ASAN)
  # Use native GCC stack smash Protection
  # and buffer overflow detection (debug only)
    add_flags(CMAKE_CXX_FLAGS_DEBUG -fstack-protector-all)
    add_flags(CMAKE_C_FLAGS_DEBUG -fstack-protector-all)
endif()


# Compilation flags for Google's ThreadSanitizer
if(VORPALINE_WITH_TSAN)
    message(FATAL_ERROR "Thread sanitizer not supported with Emscripten")
endif()

# Compilation flags for ALinea DDT
if(VORPALINE_WITH_DDT)
    message(FATAL_ERROR "Alinea DDT not supported with Emscripten")  
endif()  


# We only add the Emscripten flags here, because:
#   - they have two "-s xxxx -s yyyy" options, and
#     add_flags remove the duplicates, thus replacing
#     the two options with "-s xxxx yyyy" (the second
#     "-s" is removed)
#   - if we do add_flags() right after, this will also
#     remove the second "-s" flag.
# Thus we do that here, making sure that there is no
# call to add_flags() after.

add_flags_no_remove_duplicates(CMAKE_CXX_FLAGS_RELEASE ${EM_FLAGS_RELEASE})
add_flags_no_remove_duplicates(CMAKE_C_FLAGS_RELEASE ${EM_FLAGS_RELEASE})

add_flags_no_remove_duplicates(CMAKE_CXX_FLAGS_DEBUG ${EM_FLAGS_DEBUG})
add_flags_no_remove_duplicates(CMAKE_C_FLAGS_DEBUG ${EM_FLAGS_DEBUG})

# Reset the warning level for third parties
function(vor_reset_warning_level)
    remove_definitions(${FULL_WARNINGS})
    add_definitions(${NORMAL_WARNINGS})
endfunction()

macro(vor_add_executable)
    add_executable(${ARGN})
endmacro()


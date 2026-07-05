#-------------------------------------------------------------------
# Flags for Emscripten (javascript target)
#-------------------------------------------------------------------

# Shell script extension
set(SHELL_SUFFIX "sh")

find_path(EMSCRIPTEN_DIR
      emcc.py
      HINTS
        ENV EMSCRIPTEN
      PATHS
        "C:/Program Files/emscripten"
         /usr/lib/emscripten
	 /usr/share/emscripten
)

message(STATUS "Emscripten dir=${EMSCRIPTEN_DIR}")

set(CMAKE_C_COMPILER "emcc")
set(CMAKE_CXX_COMPILER "em++")
set(CMAKE_AR "emar")
set(CMAKE_RANLIB "emranlib")
set(CMAKE_LINKER "emld")
set(CMAKE_SKIP_RPATH TRUE)

include(${EMSCRIPTEN_DIR}/cmake/Modules/Platform/Emscripten.cmake)

set(GEOGRAM_WITH_EMSCRIPTEN TRUE)

# Warning flags
set(NORMAL_WARNINGS
    -Wall -Wextra
    -Wno-extra-semi-stmt 
    -Wno-unused-command-line-argument
    -Wno-reserved-identifier
    -Wno-format
    -Wno-unused-comparison
    -Wno-reserved-identifier
    -Wno-c++98-compat-pedantic
    -Wno-unused-but-set-variable
)

set(FULL_WARNINGS
    -Weverything
    -Wno-disabled-macro-expansion # else we got a warning each time cout is used
    -Wno-padded # Disable generating a message each time padding is used
    -Wno-float-equal # Sometimes we compare floats (against 0.0 or 1.0 mainly)
    -Wno-global-constructors
    -Wno-exit-time-destructors
    -Wno-old-style-cast # Yes, old-style cast is sometime more legible...
    -Wno-format-nonliteral # Todo: use Laurent Alonso's trick
    -Wno-extra-semi-stmt # geo_assert() in release mode creates empty stmt
    -Wno-unused-command-line-argument
    -Wno-atomic-implicit-seq-cst
    -Wno-alloca
    -Wno-reserved-identifier
    -Wno-c++98-compat-pedantic
    -Wno-unused-but-set-variable
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
  -sUSE_GLFW=3
# -sUSE_WEBGL2=1 -DGEO_WEBGL2
  -sTOTAL_MEMORY=268435456
  -sEXPORTED_FUNCTIONS='["_main","_file_system_changed_callback"]'
  -sEXPORTED_RUNTIME_METHODS='["ccall"]'
# -sNO_DISABLE_EXCEPTION_CATCHING
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
# stack protector causes undefined symbols at link time (deactivated for now).  
#    add_flags(CMAKE_CXX_FLAGS_DEBUG -fstack-protector-all)
#    add_flags(CMAKE_C_FLAGS_DEBUG -fstack-protector-all)
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

add_flags(CMAKE_EXE_LINKER_FLAGS ${EM_COMMON_FLAGS} -lnodefs.js)

# Reset the warning level for third parties
function(vor_reset_warning_level)
    remove_definitions(${FULL_WARNINGS})
    add_definitions(${NORMAL_WARNINGS})
endfunction()

macro(vor_add_executable)
    add_executable(${ARGN})
endmacro()


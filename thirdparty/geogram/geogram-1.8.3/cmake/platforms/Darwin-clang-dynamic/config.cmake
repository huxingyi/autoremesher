set(VORPALINE_ARCH_64 true)
set(VORPALINE_BUILD_DYNAMIC true)
include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Darwin-clang.cmake)
# Enable SSE3 instruction set
add_flags(CMAKE_CXX_FLAGS -m64 -msse3)
add_flags(CMAKE_C_FLAGS -m64 -msse3)

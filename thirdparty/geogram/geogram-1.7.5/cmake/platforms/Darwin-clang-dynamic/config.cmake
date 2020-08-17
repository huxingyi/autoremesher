set(VORPALINE_ARCH_64 true)
set(VORPALINE_BUILD_DYNAMIC true)
include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Darwin-clang.cmake)
add_flags(CMAKE_CXX_FLAGS -m64)
add_flags(CMAKE_C_FLAGS -m64)


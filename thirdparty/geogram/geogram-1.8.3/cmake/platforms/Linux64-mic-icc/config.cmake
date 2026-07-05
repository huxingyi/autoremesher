set(VORPALINE_ARCH_64 true)
include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Linux-mic-icc.cmake)
add_flags(CMAKE_CXX_FLAGS -mmic)
add_flags(CMAKE_C_FLAGS -mmic)
set(VORPALINE_WITH_GRAPHICS FALSE)



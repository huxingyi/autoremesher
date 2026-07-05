set(VORPALINE_ARCH_64 FALSE)
include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Emscripten-clang.cmake)

if(VORPALINE_ARCH_64)
  add_flags(CMAKE_CXX_FLAGS -m64)
  add_flags(CMAKE_C_FLAGS -m64)
endif()

set(VORPALINE_ARCH_32 true)
include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Linux-gcc.cmake)
add_flags(CMAKE_CXX_FLAGS -m32)
add_flags(CMAKE_C_FLAGS -m32)

# Configure FPU to use SSE instructions (IEEE rounding semantics)
# In the default 387 mode, rounding is unpredictable
add_definitions(-mfpmath=sse)


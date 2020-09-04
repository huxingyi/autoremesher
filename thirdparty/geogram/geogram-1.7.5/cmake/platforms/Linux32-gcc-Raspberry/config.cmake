set(VORPALINE_BUILD_DYNAMIC true)
include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Linux-gcc.cmake)

set(ARM_FLAGS -mcpu=cortex-a53  -mfpu=neon-fp-armv8 -mfloat-abi=hard -mlittle-endian -munaligned-access)

add_flags(CMAKE_CXX_FLAGS -DGEO_OS_RASPBERRY ${ARM_FLAGS})
add_flags(CMAKE_C_FLAGS -DGEO_OS_RASPBERRY ${ARM_FLAGS})


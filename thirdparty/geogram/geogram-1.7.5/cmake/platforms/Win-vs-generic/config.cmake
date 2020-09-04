if(CMAKE_CL_64)
set(VORPALINE_ARCH_64 true)
message(STATUS "Configuring 64 bits build")
else()
set(VORPALINE_ARCH_32 true)
message(STATUS "Configuring 32 bits build")
endif()

include(${GEOGRAM_SOURCE_DIR}/cmake/platforms/Windows-vs.cmake)

find_path(SNAPPY_INCLUDE_DIR snappy-c.h)

find_library(SNAPPY_LIBRARY NAMES snappy)

if (SNAPPY_INCLUDE_DIR AND SNAPPY_LIBRARY)
    set(SNAPPY_FOUND TRUE)
    message(STATUS "Found SNAPPY library: ${SNAPPY_LIBRARY}")
else ()
    message(STATUS "No snappy found.  Using internal sources.")
endif ()

vor_reset_warning_level()

#Disable all warnings (for this generated code, we got too many of them)
if(WIN32)
    remove_definitions(/W3)
    add_definitions(-w)
else()
    add_definitions(-w)
endif()

include_directories(${GEOGRAM_SOURCE_DIR}/src/lib/third_party/numerics/INCLUDE)

aux_source_directories(SOURCES "Source Files" .)
aux_source_directories(SOURCES "Source Files\\LIBF2C" LIBF2C)
aux_source_directories(SOURCES "Source Files\\CBLAS" CBLAS)
aux_source_directories(SOURCES "Source Files\\CLAPACK" CLAPACK)
aux_source_directories(SOURCES "Source Files\\SUPERLU" SUPERLU)
aux_source_directories(SOURCES "Source Files\\ARPACK" ARPACK)
aux_source_directories(SOURCES "Source Files\\ARPACK_UTIL" ARPACK_UTIL)

add_library(geogram_num_3rdparty ${SOURCES})

if(UNIX)
    target_link_libraries(geogram_num_3rdparty m)
endif()

set_target_properties(geogram_num_3rdparty PROPERTIES
                      VERSION ${VORPALINE_VERSION}
                      SOVERSION ${VORPALINE_VERSION_MAJOR}
		      FOLDER "GEOGRAM")

install_devkit_targets(geogram_num_3rdparty)
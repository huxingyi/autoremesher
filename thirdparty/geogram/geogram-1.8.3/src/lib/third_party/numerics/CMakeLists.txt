vor_reset_warning_level()

#Disable all warnings (for this generated code, we got too many of them)
if(WIN32)
#  code below did not work:
#    remove_definitions(/W3)
#    add_definitions(/W0)
#  so I'm doing that instead (I know it is not good CMake practice,
#  but I know no other option...)
    string(REGEX REPLACE "/W[0-4]" "/W0" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
else()
  add_definitions(-w -fcommon)
endif()

include_directories(${GEOGRAM_SOURCE_DIR}/src/lib/third_party/numerics/INCLUDE)

aux_source_directories(SOURCES "Source Files" .)
aux_source_directories(SOURCES "Source Files\\LIBF2C" LIBF2C)
aux_source_directories(SOURCES "Source Files\\CBLAS" CBLAS)
aux_source_directories(SOURCES "Source Files\\CLAPACK" CLAPACK)
aux_source_directories(SOURCES "Source Files\\SUPERLU" SUPERLU)
aux_source_directories(SOURCES "Source Files\\ARPACK" ARPACK)
aux_source_directories(SOURCES "Source Files\\ARPACK_UTIL" ARPACK_UTIL)

# This file uses system() which is not available on iOS. The caller function is not used elsewhere
# so just remove source file from the set of files to compile
list(REMOVE_ITEM SOURCES "LIBF2C/system_.c")

add_library(geogram_num_3rdparty ${SOURCES})

if(UNIX)
    target_link_libraries(geogram_num_3rdparty m)
endif()

set_target_properties(geogram_num_3rdparty PROPERTIES
                      VERSION ${VORPALINE_VERSION}
                      SOVERSION ${VORPALINE_VERSION_MAJOR}
		      FOLDER "GEOGRAM")

install_devkit_targets(geogram_num_3rdparty)

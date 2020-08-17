
aux_source_directories(SOURCES "Source Files"                    .)
aux_source_directories(SOURCES "Source Files\\hexdom"            hexdom)
aux_source_directories(SOURCES "Source Files\\optimal_transport" optimal_transport)


include_directories(${PROJECT_BINARY_DIR}/src/lib)

if(WIN32)
    add_library(exploragram SHARED ${SOURCES} $<TARGET_OBJECTS:geogram>)
else()
    add_library(exploragram ${SOURCES})
endif()

set_target_properties(exploragram PROPERTIES
                      VERSION ${VORPALINE_VERSION}
                      SOVERSION ${VORPALINE_VERSION_MAJOR}
		      FOLDER "GEOGRAM")

if(UNIX AND VORPALINE_BUILD_DYNAMIC)
    target_link_libraries(exploragram pthread dl)
endif()

if(WIN32)
    target_link_libraries(exploragram psapi)
endif()

if(GEOGRAM_WITH_VORPALINE)
  target_link_libraries(exploragram vorpalib)
endif()

if(WIN32)
else()
    target_link_libraries(exploragram geogram)
endif()

set_target_properties(
   exploragram PROPERTIES
   FOLDER "GEOGRAM")


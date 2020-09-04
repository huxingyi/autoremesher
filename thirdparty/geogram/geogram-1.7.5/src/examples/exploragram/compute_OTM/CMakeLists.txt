set(APP_NAME compute_OTM)

aux_source_directories(SOURCES "" .)
vor_add_executable(${APP_NAME} ${SOURCES})

if(GEOGRAM_WITH_VORPALINE)
  add_definitions(-DGEOGRAM_WITH_VORPALINE)
  target_link_libraries(${APP_NAME} exploragram geogram vorpalib)
else()
  target_link_libraries(${APP_NAME} exploragram geogram)
endif()

set_target_properties(${APP_NAME} PROPERTIES FOLDER "GEOGRAM/Examples")

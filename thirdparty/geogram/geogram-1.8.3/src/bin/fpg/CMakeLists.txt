vor_reset_warning_level()
set(APP_NAME mcc)
aux_source_directories(SOURCES "" .)
include_directories(.)
vor_add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} geogram)

set_target_properties(${APP_NAME} PROPERTIES FOLDER "GEOGRAM/Programs")

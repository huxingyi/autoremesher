#
# Build rules for the documentation
#

include(${PROJECT_SOURCE_DIR}/cmake/doxygen.cmake)

cmake_minimum_required(VERSION 2.8)



if(DOXYGEN_FOUND)

  # Add a custom documentation targets

  add_doc_target(devkit)
  add_doc_target(devkit-full)
  add_doc_target(devkit-internal)
  add_doc_target(devkit-internal-light)

  # TODO: add end-user documentation

  # Copy the Version info file to the build directory
  configure_file(VERSION.txt.in VERSION.txt @ONLY)

  # Install documentation
  if(GEOGRAM_WITH_VORPALINE)
      install(FILES README.txt DESTINATION doc COMPONENT runtime)
      install(FILES ${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt DESTINATION doc COMPONENT runtime OPTIONAL)
  endif()

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/VERSION.txt DESTINATION doc/geogram COMPONENT runtime OPTIONAL)
  
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/devkit/html DESTINATION doc/devkit COMPONENT doc-devkit OPTIONAL)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/devkit-full/html DESTINATION doc/devkit COMPONENT doc-devkit-full OPTIONAL)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/devkit-internal/html DESTINATION doc/devkit COMPONENT doc-devkit-internal OPTIONAL)

endif()

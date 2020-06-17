include (ACGDoxygen)

IF (DOXYGEN_FOUND)

  # Add a documentation install target
  add_custom_target(doc-install COMMENT "Installing Documentation" VERBATIM )

  # Build Documentation before installing it
  add_dependencies(doc-install doc)

  if (TARGET doc-install)
    acg_copy_after_build (doc-install "${CMAKE_BINARY_DIR}/Build/${ACG_PROJECT_DATADIR}/Doc/html"   "${CMAKE_INSTALL_PREFIX}/${ACG_PROJECT_DATADIR}/doc/html")
  endif()

ENDIF(DOXYGEN_FOUND)

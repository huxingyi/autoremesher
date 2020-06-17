include (CoMISoExample)

acg_add_executable (small_sparseqr ${sources} ${headers} )

# enable rpath linking
set_target_properties(small_sparseqr PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1)

target_link_libraries (small_sparseqr
  CoMISo
  ${COMISO_LINK_LIBRARIES}
)

include (CoMISoExample)

acg_add_executable (small_nleast_squares ${sources} ${headers} )

# enable rpath linking
set_target_properties(small_nleast_squares PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1)

target_link_libraries (small_nleast_squares
  CoMISo
  ${COMISO_LINK_LIBRARIES}
)

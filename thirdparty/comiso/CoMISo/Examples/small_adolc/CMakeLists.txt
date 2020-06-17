include (CoMISoExample)

acg_add_executable (small_adolc ${sources} ${headers} )

# enable rpath linking
set_target_properties(small_adolc PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1)

target_link_libraries (small_adolc
  CoMISo
  ${COMISO_LINK_LIBRARIES}
)

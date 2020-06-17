include (CoMISoExample)

acg_add_executable (small_linear_problem ${sources} ${headers} )

# enable rpath linking
set_target_properties(small_linear_problem PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1)

target_link_libraries (small_linear_problem
  CoMISo
  ${COMISO_LINK_LIBRARIES}
)

include (CoMISoExample)

acg_add_executable (small_quadratic_solver ${sources} ${headers} )

# enable rpath linking
set_target_properties(small_quadratic_solver PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1)

target_link_libraries (small_quadratic_solver
  CoMISo
  ${COMISO_LINK_LIBRARIES}
)

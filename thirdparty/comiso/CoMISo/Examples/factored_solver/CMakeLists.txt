include (CoMISoExample)

acg_add_executable (factored_solver ${sources} ${headers} )

# enable rpath linking
set_target_properties(factored_solver PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1)

target_link_libraries (factored_solver
  CoMISo
  ${COMISO_LINK_LIBRARIES}
)

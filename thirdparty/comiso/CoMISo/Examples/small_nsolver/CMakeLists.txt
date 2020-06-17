include (CoMISoExample)

acg_add_executable (small_nsolver ${sources} ${headers} )

# enable rpath linking
set_target_properties(small_nsolver PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1)

target_link_libraries (small_nsolver
  CoMISo
  ${COMISO_LINK_LIBRARIES}
)

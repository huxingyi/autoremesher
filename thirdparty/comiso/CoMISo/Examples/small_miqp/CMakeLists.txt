include (CoMISoExample)

acg_add_executable (small_miqp ${sources} ${headers} )

# enable rpath linking
set_target_properties(small_miqp PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1)

target_link_libraries (small_miqp
  CoMISo
  ${COMISO_LINK_LIBRARIES}
)

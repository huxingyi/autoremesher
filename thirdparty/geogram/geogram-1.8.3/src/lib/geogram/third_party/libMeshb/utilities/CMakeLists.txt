
#################
# BUILD UTILITIES
#################

add_executable(transmesh transmesh.c ../sources/libmeshb7.c)
target_compile_options (transmesh PRIVATE -DTRANSMESH)
target_link_libraries (transmesh Meshb.7 ${AIO_LIBRARIES})
install (TARGETS transmesh DESTINATION bin COMPONENT applications)

add_executable(mshinfo mshinfo.c)
target_link_libraries (mshinfo Meshb.7 ${AIO_LIBRARIES})
install (TARGETS mshinfo DESTINATION bin COMPONENT applications)

add_executable(speed_test speed_test.c)
target_link_libraries (speed_test Meshb.7 ${AIO_LIBRARIES})
install (TARGETS speed_test DESTINATION bin COMPONENT applications)

add_executable(ugrid2mesh ugrid2mesh.c)
target_link_libraries (ugrid2mesh Meshb.7 ${AIO_LIBRARIES})
install (TARGETS ugrid2mesh DESTINATION bin COMPONENT applications)

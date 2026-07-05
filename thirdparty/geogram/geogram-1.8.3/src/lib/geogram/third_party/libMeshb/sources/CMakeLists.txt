
##########################
# BUILD THE STATIC LIBRARY
##########################

add_library(Meshb.7 libmeshb7.c ../utilities/libmeshb7_helpers.c)
install (FILES libmeshb7.h ../utilities/libmeshb7_helpers.h DESTINATION include COMPONENT headers)
target_include_directories(
  Meshb.7 PUBLIC $<INSTALL_INTERFACE:include>
                 $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/sources>)
install (TARGETS Meshb.7 EXPORT meshb-target DESTINATION lib COMPONENT libraries)
install (EXPORT meshb-target NAMESPACE ${PROJECT_NAME}::
         DESTINATION lib/cmake/${PROJECT_NAME})
install (TARGETS Meshb.7 EXPORT libMeshb-target DESTINATION lib COMPONENT libraries)
install (EXPORT libMeshb-target DESTINATION lib/cmake/${PROJECT_NAME})
export  (PACKAGE libMeshb)


####################################
# BUILD THE LIBRARY WITH FORTRAN API
####################################

if (CMAKE_Fortran_COMPILER AND WITH_GMF_FORTRAN)
   file(GLOB_RECURSE SOURCES *.[chfF] *.[fF]90)
   add_library(Meshbf.7 STATIC ${SOURCES})
   set_target_properties(Meshbf.7 PROPERTIES COMPILE_FLAGS "-DF77API")
   target_include_directories(Meshbf.7
                              INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
                              PRIVATE   ${CMAKE_CURRENT_BINARY_DIR}
                              PRIVATE   ${CMAKE_BINARY_DIR}
                              PUBLIC    ${CMAKE_SOURCE_DIR} )
   
   install(FILES libmeshb7.ins DESTINATION include)
   
   install(TARGETS Meshbf.7
           RUNTIME DESTINATION bin
           LIBRARY DESTINATION lib
           PUBLIC_HEADER DESTINATION include
           ARCHIVE DESTINATION lib)

endif ()

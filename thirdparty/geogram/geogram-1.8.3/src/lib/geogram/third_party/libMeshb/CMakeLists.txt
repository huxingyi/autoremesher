

################################
# GLOBAL CONFIGURATION VARIABLES
################################

cmake_minimum_required (VERSION 3.7.2)
set (libMeshb_VERSION_MAJOR 7)
set (libMeshb_VERSION_MINOR 54)
project(libMeshb VERSION ${libMeshb_VERSION_MAJOR}.${libMeshb_VERSION_MINOR} LANGUAGES C)

option(WITH_GMF_AIO     "Use Unix low-level and asynchronous I/O for higher speed"       OFF)
option(WITH_GMF_CPACK   "Enable cpack target to generate a zip file containing binaries" OFF)
option(WITH_GMF_FORTRAN "Build the Fortran API"                                          ON )

include (CheckLanguage)
check_language (Fortran)
if(CMAKE_Fortran_COMPILER AND WITH_GMF_FORTRAN)
  enable_language(Fortran)
  set(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/modules)
  if (CMAKE_Fortran_COMPILER_ID STREQUAL GNU)
     set (CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -std=legacy")
  endif ()
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

if (CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
   set (math_LIBRARIES "m")
endif ()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   if (WIN32)
      set(CMAKE_INSTALL_PREFIX "$ENV{HOMEPATH}/cmakebuilds" CACHE PATH "..." FORCE)
   else ()
      if (DEFINED ENV{OSTYPE} AND DEFINED ENV{MACHTYPE})
         set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/cmakebuilds/$ENV{OSTYPE}-$ENV{MACHTYPE}" CACHE PATH "..." FORCE)
      else ()
         set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/cmakebuilds" CACHE PATH "..." FORCE)
      endif ()
   endif()
endif ()

list (APPEND CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}")

if(WITH_GMF_AIO)
   set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITH_GMF_AIO")
   IF(${CMAKE_SYSTEM_NAME} MATCHES Linux)
      set(AIO_LIBRARIES rt)
   endif()
endif()


#######################################
# SET FILES AND DIRECTORIES TO BE BUILT
#######################################

include_directories (${PROJECT_SOURCE_DIR}/sources)
include_directories (${PROJECT_SOURCE_DIR}/utilities)
add_subdirectory (sources)
add_subdirectory (examples)
add_subdirectory (utilities)

install (FILES LICENSE.txt copyright.txt DESTINATION share/libMeshb)
install (DIRECTORY sample_meshes DESTINATION share/libMeshb)


######################################
# SET PACKAGE AND DEPLOYMENT VARIABLES
######################################

if (WITH_GMF_CPACK)
   include (InstallRequiredSystemLibraries)
   set (CPACK_GENERATOR TXZ)
   set (CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/copyright.txt)
   set (CPACK_PACKAGE_VERSION_MAJOR ${libMeshb_VERSION_MAJOR})
   set (CPACK_PACKAGE_VERSION_MINOR ${libMeshb_VERSION_MINOR})
   set (CPACK_COMPONENTS_ALL applications examples libraries headers)
   include (CPack)
endif ()


##################################
# EXPORT CMAKE PACKAGE INFORMATION
##################################

include(CMakePackageConfigHelpers)
file(WRITE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
"include(\${CMAKE_CURRENT_LIST_DIR}/meshb-target.cmake)
include(\${CMAKE_CURRENT_LIST_DIR}/libMeshb-target.cmake)
set(libMeshb_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
set(libMeshb_LINK_DIRS ${CMAKE_INSTALL_PREFIX}/lib)
set(libMeshb_LIBRARIES Meshb.7 ${AIO_LIBRARIES})
set(libMeshb_Fortran_LIBRARIES Meshbf.7 ${AIO_LIBRARIES})
set(libMeshb_FOUND TRUE)
")
write_basic_package_version_file(
   "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
   VERSION ${PROJECT_VERSION}
   COMPATIBILITY SameMajorVersion)
install(FILES
   "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
   "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
   DESTINATION lib/cmake/${PROJECT_NAME})


##################################
# PRINT OPERATIONS TO BE CONDUCTED
##################################

message("-- Build mode            : " ${CMAKE_BUILD_TYPE})
message("-- cpack target enabled  : " ${WITH_GMF_CPACK})
message("-- Asynchronous IO       : " ${WITH_GMF_AIO})
message("-- Fortran API           : " ${WITH_GMF_FORTRAN})
message("-- Install directory     : " ${CMAKE_INSTALL_PREFIX})

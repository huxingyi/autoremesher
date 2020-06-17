# - Try to find EIGEN3
# Once done this will define
#  EIGEN3_FOUND         - System has EIGEN3
#  EIGEN3_INCLUDE_DIRS  - The EIGEN3 include directories

if (EIGEN3_INCLUDE_DIR)
  # in cache already
  set(EIGEN3_FOUND TRUE)
  set(EIGEN3_INCLUDE_DIRS "${EIGEN3_INCLUDE_DIR}" )
else (EIGEN3_INCLUDE_DIR)

# Check if the base path is set
if ( NOT CMAKE_WINDOWS_LIBS_DIR )
  # This is the base directory for windows library search used in the finders we shipp.
  set(CMAKE_WINDOWS_LIBS_DIR "c:/libs" CACHE STRING "Default Library search dir on windows." )
endif()

if ( CMAKE_GENERATOR MATCHES "^Visual Studio 11.*Win64" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2012/x64/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 11.*" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2012/x32/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 12.*Win64" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2013/x64/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 12.*" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2013/x32/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 14.*Win64" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2015/x64/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 14.*" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2015/x32/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 15.*Win64" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2017/x64/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 15.*" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2017/x32/")
endif()


find_path( EIGEN3_INCLUDE_DIR 
	   NAMES Eigen/Dense 
           PATHS $ENV{EIGEN_DIR}
                 /usr/include/eigen3
                 /usr/local/include
                 /usr/local/include/eigen3/
                 /opt/local/include/eigen3/
                 "${CMAKE_WINDOWS_LIBS_DIR}/general/Eigen-3.3.4"
                 "${CMAKE_WINDOWS_LIBS_DIR}/general/Eigen-3.2.8"
                 "${CMAKE_WINDOWS_LIBS_DIR}/general/Eigen-3.2.6"
                 "${CMAKE_WINDOWS_LIBS_DIR}/Eigen-3.2.6"
                 "${CMAKE_WINDOWS_LIBS_DIR}/Eigen-3.2.6/include"
                 "${CMAKE_WINDOWS_LIBS_DIR}/Eigen-3.2.1"
                 "${CMAKE_WINDOWS_LIBS_DIR}/Eigen-3.2.1/include"
                 "${CMAKE_WINDOWS_LIBS_DIR}/Eigen-3.2/include"
                 "${CMAKE_WINDOWS_LIBS_DIR}/eigen3/include"
                 "${CMAKE_WINDOWS_LIBS_DIR}/eigen/include"
                 ${PROJECT_SOURCE_DIR}/MacOS/Libs/eigen3/include
                 ../../External/include
                 ${module_file_path}/../../../External/include
          )

set(EIGEN3_INCLUDE_DIRS "${EIGEN3_INCLUDE_DIR}" )


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBCPLEX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(EIGEN3  DEFAULT_MSG
                                  EIGEN3_INCLUDE_DIR)

mark_as_advanced(EIGEN3_INCLUDE_DIR)

endif(EIGEN3_INCLUDE_DIR)

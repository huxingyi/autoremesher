# - Try to find CBC
# Once done this will define
#  CBC_FOUND - System has CBC
#  CBC_INCLUDE_DIRS - The CBC include directories
#  CBC_LIBRARIES - The libraries needed to use CBC

# I8 Search paths for windows libraries
if ( CMAKE_GENERATOR MATCHES "^Visual Studio 11.*Win64" )
  SET(VS_SEARCH_PATH "c:/libs/vs2012/x64/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 11.*" )
  SET(VS_SEARCH_PATH "c:/libs/vs2012/x32/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 12.*Win64" )
  SET(VS_SEARCH_PATH "c:/libs/vs2013/x64/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 12.*" )
  SET(VS_SEARCH_PATH "c:/libs/vs2013/x32/")
endif()

if (CBC_INCLUDE_DIR)
  # in cache already
  set(CBC_FOUND TRUE)
  set(CBC_INCLUDE_DIRS "${CBC_INCLUDE_DIR}" )
  set(CBC_LIBRARIES "${CBC_LIBRARY};${CBC_SOLVER_LIBRARY}" )
else (CBC_INCLUDE_DIR)


find_path(CBC_INCLUDE_DIR 
          NAMES CbcConfig.h
          PATHS "$ENV{CBC_DIR}/include/coin"
                 "/usr/include/coin"
                 "C:\\libs\\cbc\\include"
				 "${VS_SEARCH_PATH}CBC-2.9.4/Cbc/include"
          )

find_library( CBC_LIBRARY 
              NAMES Cbc libCbc
              PATHS "$ENV{CBC_DIR}/lib" 
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\cbc\\lib"
					"${VS_SEARCH_PATH}CBC-2.9.4/Cbc/lib"
              )

find_library( CBC_SOLVER_LIBRARY 
              NAMES CbcSolver libCbcSolver

              PATHS "$ENV{CBC_DIR}/lib" 
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\cbc\\lib"
					"${VS_SEARCH_PATH}CBC-2.9.4/Cbc/lib"
              )

set(CBC_INCLUDE_DIRS "${CBC_INCLUDE_DIR}" )
set(CBC_LIBRARIES "${CBC_LIBRARY};${CBC_SOLVER_LIBRARY}" )


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set CBC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(CBC  DEFAULT_MSG
                                  CBC_LIBRARY CBC_SOLVER_LIBRARY CBC_INCLUDE_DIR)

mark_as_advanced(CBC_INCLUDE_DIR CBC_LIBRARY CBC_SOLVER_LIBRARY)

endif(CBC_INCLUDE_DIR)

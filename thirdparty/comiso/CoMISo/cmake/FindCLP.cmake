# - Try to find CLP
# Once done this will define
#  CLP_FOUND - System has CLP
#  CLP_INCLUDE_DIRS - The CLP include directories
#  CLP_LIBRARIES - The libraries needed to use CLP


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

if (CLP_INCLUDE_DIR)
  # in cache already
  set(CLP_FOUND TRUE)
  set(CLP_INCLUDE_DIRS "${CLP_INCLUDE_DIR}" )
  set(CLP_LIBRARIES "${CLP_LIBRARY}" )
else (CLP_INCLUDE_DIR)

find_path(CLP_INCLUDE_DIR 
          NAMES ClpConfig.h
          PATHS "$ENV{CLP_DIR}/include/coin"
                "$ENV{CBC_DIR}/include/coin"
                 "/usr/include/coin"
                 "C:\\libs\\clp\\include"
                 "C:\\libs\\cbc\\include"
				 "${VS_SEARCH_PATH}CBC-2.9.4/Clp/include"
          )

find_library( CLP_LIBRARY 
              NAMES Clp libClp
              PATHS "$ENV{CLP_DIR}/lib"
                    "$ENV{CBC_DIR}/lib" 
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\clp\\lib"
                    "C:\\libs\\cbc\\lib"
					"${VS_SEARCH_PATH}CBC-2.9.4/Clp/lib"
              )

set(CLP_INCLUDE_DIRS "${CLP_INCLUDE_DIR}" )
set(CLP_LIBRARIES "${CLP_LIBRARY}" )


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set CLP_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(CLP  DEFAULT_MSG
                                  CLP_LIBRARY CLP_INCLUDE_DIR)

mark_as_advanced(CLP_INCLUDE_DIR CLP_LIBRARY)

endif(CLP_INCLUDE_DIR)

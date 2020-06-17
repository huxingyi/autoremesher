# - Try to find COINUTILS
# Once done this will define
#  COINUTILS_FOUND - System has COINUTILS
#  COINUTILS_INCLUDE_DIRS - The COINUTILS include directories
#  COINUTILS_LIBRARIES - The libraries needed to use COINUTILS

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

if (COINUTILS_INCLUDE_DIR)
  # in cache already
  set(COINUTILS_FOUND TRUE)
  set(COINUTILS_INCLUDE_DIRS "${COINUTILS_INCLUDE_DIR}" )
  set(COINUTILS_LIBRARIES "${COINUTILS_LIBRARY}" )
else (COINUTILS_INCLUDE_DIR)

find_path(COINUTILS_INCLUDE_DIR 
          NAMES CoinUtilsConfig.h
          PATHS "$ENV{COINUTILS_DIR}/include/coin"
                "$ENV{CBC_DIR}/include/coin"
                 "/usr/include/coin"
                 "C:\\libs\\coinutils\\include"
                 "C:\\libs\\cbc\\include"
				 "${VS_SEARCH_PATH}CBC-2.9.4/CoinUtils/include"
          )

find_library( COINUTILS_LIBRARY 
              NAMES CoinUtils libCoinUtils
              PATHS "$ENV{COINUTILS_DIR}/lib"
                    "$ENV{CBC_DIR}/lib" 
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\coinutils\\lib"
                    "C:\\libs\\cbc\\lib"
					"${VS_SEARCH_PATH}CBC-2.9.4/CoinUtils/lib"
              )

set(COINUTILS_INCLUDE_DIRS "${COINUTILS_INCLUDE_DIR}" )
set(COINUTILS_LIBRARIES "${COINUTILS_LIBRARY}" )


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set COINUTILS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(COINUTILS  DEFAULT_MSG
                                  COINUTILS_LIBRARY COINUTILS_INCLUDE_DIR)

mark_as_advanced(COINUTILS_INCLUDE_DIR COINUTILS_LIBRARY)

endif(COINUTILS_INCLUDE_DIR)

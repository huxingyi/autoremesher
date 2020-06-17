# - Try to find OSI
# Once done this will define
#  OSI_FOUND - System has OSI
#  OSI_INCLUDE_DIRS - The OSI include directories
#  OSI_LIBRARIES - The libraries needed to use OSI

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


if (OSI_INCLUDE_DIR)
  # in cache already
  set(OSI_FOUND TRUE)
  set(OSI_INCLUDE_DIRS "${OSI_INCLUDE_DIR}" )
  set(OSI_LIBRARIES "${OSI_LIBRARY};${OSI_CBC_LIBRARY};${OSI_CLP_LIBRARY}" )
else (OSI_INCLUDE_DIR)

find_path(OSI_INCLUDE_DIR 
          NAMES OsiConfig.h
		  PATH_SUFFIXES "Osi"
          PATHS "$ENV{OSI_DIR}/include/coin"
                "$ENV{CBC_DIR}/include/coin"
                 "/usr/include/coin"
                 "C:\\libs\\osi\\include"
                 "C:\\libs\\cbc\\include"
				 "${VS_SEARCH_PATH}CBC-2.9.4/Osi/include"
          )

find_library( OSI_LIBRARY 
              NAMES Osi libOsi
              PATHS "$ENV{OSI_DIR}/lib"
                    "$ENV{CBC_DIR}/lib" 
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\OSI\\lib"
                    "C:\\libs\\cbc\\lib"
					"${VS_SEARCH_PATH}CBC-2.9.4/Osi/lib"
              )

find_library( OSI_CBC_LIBRARY 
              NAMES OsiCbc libOsiCbc
              PATHS "$ENV{OSI_DIR}/lib"
                    "$ENV{CBC_DIR}/lib" 
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\OSI\\lib"
                    "C:\\libs\\cbc\\lib"
					"${VS_SEARCH_PATH}CBC-2.9.4/Osi/lib"
              )

find_library( OSI_CLP_LIBRARY 
              NAMES OsiClp libOsiClp
              PATHS "$ENV{OSI_DIR}/lib"
                    "$ENV{CBC_DIR}/lib" 
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\OSI\\lib"
                    "C:\\libs\\cbc\\lib"
					"${VS_SEARCH_PATH}CBC-2.9.4/Osi/lib"
              )

set(OSI_INCLUDE_DIRS "${OSI_INCLUDE_DIR}" )
set(OSI_LIBRARIES "${OSI_LIBRARY};${OSI_CBC_LIBRARY};${OSI_CLP_LIBRARY}" )


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set OSI_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(OSI  DEFAULT_MSG
                                  OSI_LIBRARY OSI_CBC_LIBRARY OSI_CLP_LIBRARY OSI_INCLUDE_DIR)

mark_as_advanced(OSI_INCLUDE_DIR OSI_LIBRARY OSI_CBC_LIBRARY OSI_CLP_LIBRARY)

endif(OSI_INCLUDE_DIR)

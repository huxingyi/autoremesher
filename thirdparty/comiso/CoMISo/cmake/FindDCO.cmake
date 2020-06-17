# - Try to find DCO
# Once done this will define
#  DCO_FOUND         - System has DCO
#  DCO_INCLUDE_DIRS  - The DCO include directories

if (DCO_INCLUDE_DIR)
  # in cache already
  set(DCO_FOUND TRUE)
  set(DCO_INCLUDE_DIRS "${DCO_INCLUDE_DIR}" )
else (DCO_INCLUDE_DIR)

find_path( DCO_INCLUDE_DIR 
           NAMES dco.hpp
           PATHS $ENV{DCO_DIR}
                 /usr/include/dco
                 /usr/local/include
                 /usr/local/include/dco/
                 /opt/local/include/dco/
          )

set(DCO_INCLUDE_DIRS "${DCO_INCLUDE_DIR}" )


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set DCO_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(DCO  DEFAULT_MSG
                                  DCO_INCLUDE_DIR)

mark_as_advanced(DCO_INCLUDE_DIR)

endif(DCO_INCLUDE_DIR)

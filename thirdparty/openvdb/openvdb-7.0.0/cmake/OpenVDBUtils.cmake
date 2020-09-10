# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[.rst:

OpenVDBUtils.cmake
------------------

A utility CMake file which provides helper functions for configuring an
OpenVDB installation.

Use this module by invoking include with the form::

  include ( OpenVDBUtils )


The following functions are provided:

``OPENVDB_VERSION_FROM_HEADER``

  OPENVDB_VERSION_FROM_HEADER ( <header_path>
    VERSION [<version>]
    MAJOR   [<version>]
    MINOR   [<version>]
    PATCH   [<version>] )

  Parse the provided version file to retrieve the current OpenVDB
  version information. The file is expected to be a version.h file
  as found in the following path of an OpenVDB repository:
    openvdb/version.h

  If the file does not exist, variables are unmodified.

``OPENVDB_ABI_VERSION_FROM_PRINT``

  OPENVDB_ABI_VERSION_FROM_PRINT ( <vdb_print>
    [QUIET]
    ABI [<version>] )

  Retrieve the ABI version that an installation of OpenVDB was compiled
  for using the provided vdb_print binary. Parses the result of:
    vdb_print --version

  If the binary does not exist or fails to launch, variables are
  unmodified.

#]=======================================================================]

cmake_minimum_required(VERSION 3.3)


function(OPENVDB_VERSION_FROM_HEADER OPENVDB_VERSION_FILE)
  cmake_parse_arguments(_VDB "" "VERSION;MAJOR;MINOR;PATCH" "" ${ARGN})

  if(NOT EXISTS ${OPENVDB_VERSION_FILE})
    return()
  endif()

  file(STRINGS "${OPENVDB_VERSION_FILE}" openvdb_version_str
    REGEX "^#define[\t ]+OPENVDB_LIBRARY_MAJOR_VERSION_NUMBER[\t ]+.*"
  )
  string(REGEX REPLACE "^.*OPENVDB_LIBRARY_MAJOR_VERSION_NUMBER[\t ]+([0-9]*).*$" "\\1"
    _OpenVDB_MAJOR_VERSION "${openvdb_version_str}"
  )

  file(STRINGS "${OPENVDB_VERSION_FILE}" openvdb_version_str
    REGEX "^#define[\t ]+OPENVDB_LIBRARY_MINOR_VERSION_NUMBER[\t ]+.*"
  )
  string(REGEX REPLACE "^.*OPENVDB_LIBRARY_MINOR_VERSION_NUMBER[\t ]+([0-9]*).*$" "\\1"
    _OpenVDB_MINOR_VERSION "${openvdb_version_str}"
  )

  file(STRINGS "${OPENVDB_VERSION_FILE}" openvdb_version_str
    REGEX "^#define[\t ]+OPENVDB_LIBRARY_PATCH_VERSION_NUMBER[\t ]+.*"
  )
  string(REGEX REPLACE "^.*OPENVDB_LIBRARY_PATCH_VERSION_NUMBER[\t ]+([0-9]*).*$" "\\1"
    _OpenVDB_PATCH_VERSION "${openvdb_version_str}"
  )
  unset(openvdb_version_str)

  if(_VDB_VERSION)
    set(${_VDB_VERSION}
      ${_OpenVDB_MAJOR_VERSION}.${_OpenVDB_MINOR_VERSION}.${_OpenVDB_PATCH_VERSION}
      PARENT_SCOPE
    )
  endif()
  if(_VDB_MAJOR)
    set(${_VDB_MAJOR} ${_OpenVDB_MAJOR_VERSION} PARENT_SCOPE)
  endif()
  if(_VDB_MINOR)
    set(${_VDB_MINOR} ${_OpenVDB_MINOR_VERSION} PARENT_SCOPE)
  endif()
  if(_VDB_PATCH)
    set(${_VDB_PATCH} ${_OpenVDB_PATCH_VERSION} PARENT_SCOPE)
  endif()
endfunction()


########################################################################
########################################################################


function(OPENVDB_ABI_VERSION_FROM_PRINT OPENVDB_PRINT)
  cmake_parse_arguments(_VDB "QUIET" "ABI" "" ${ARGN})

  if(NOT EXISTS ${OPENVDB_PRINT})
    return()
  endif()

  set(_VDB_PRINT_VERSION_STRING "")
  set(_VDB_PRINT_RETURN_STATUS "")

  if(${_VDB_QUIET})
    execute_process(COMMAND ${OPENVDB_PRINT} "--version"
      RESULT_VARIABLE _VDB_PRINT_RETURN_STATUS
      OUTPUT_VARIABLE _VDB_PRINT_VERSION_STRING
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  else()
    execute_process(COMMAND ${OPENVDB_PRINT} "--version"
      RESULT_VARIABLE _VDB_PRINT_RETURN_STATUS
      OUTPUT_VARIABLE _VDB_PRINT_VERSION_STRING
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  endif()

  if(${_VDB_PRINT_RETURN_STATUS})
    return()
  endif()

  set(_OpenVDB_ABI)
  string(REGEX REPLACE ".*abi([0-9]*).*" "\\1" _OpenVDB_ABI ${_VDB_PRINT_VERSION_STRING})
  unset(_VDB_PRINT_RETURN_STATUS)
  unset(_VDB_PRINT_VERSION_STRING)

  if(_VDB_ABI)
    set(${_VDB_ABI} ${_OpenVDB_ABI} PARENT_SCOPE)
  endif()
endfunction()

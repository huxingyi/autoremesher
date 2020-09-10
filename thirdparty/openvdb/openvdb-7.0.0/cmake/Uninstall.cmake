# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[.rst:

Uninstall
---------

Adds a custom target to the CMake build generation which allows for
calling::

  make uninstall

to remove an installation of OpenVDB. Relies on the install_manifest
existing from a previous run of cmake.

#]=======================================================================]

cmake_minimum_required(VERSION 3.3)

set(MANIFEST "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")

if(NOT EXISTS ${MANIFEST})
  message(FATAL_ERROR "Cannot find install manifest: '${MANIFEST}'")
endif()

file(STRINGS ${MANIFEST} INSTALLED_FILES)
foreach(INSTALLED_FILE ${INSTALLED_FILES})
  if(EXISTS ${INSTALLED_FILE})
    message(STATUS "Uninstalling: ${INSTALLED_FILE}")
    exec_program(
       ${CMAKE_COMMAND} ARGS "-E remove ${INSTALLED_FILE}"
       OUTPUT_VARIABLE stdout
       RETURN_VALUE RESULT
    )

    if(NOT "${RESULT}" STREQUAL 0)
      message(FATAL_ERROR "Failed to remove file: '${INSTALLED_FILE}'.")
    endif()
  endif()
endforeach(INSTALLED_FILE)

# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

# we include this first to parse configure.ac and extract the version
# numbers
include(config/ParseConfigure.cmake)

project(OpenEXR_Viewers VERSION ${OPENEXR_VIEWERS_VERSION})

#######################################
#######################################
# This declares all the configuration variables visible
# in cmake-gui or similar and the rest of the global
# project setup
#
# Please look at this file to see what is configurable
#######################################
#######################################
include(config/OpenEXRViewersSetup.cmake)

find_package(IlmBase ${OPENEXR_VIEWERS_VERSION} EXACT REQUIRED CONFIG)
find_package(OpenEXR ${OPENEXR_VIEWERS_VERSION} EXACT REQUIRED CONFIG)

# This is for newer cmake versions who know about vendor versions
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL)

set(FLTK_SKIP_FLUID 1)
find_package(FLTK)
if(FLTK_FOUND AND TARGET OpenGL::GL)
  add_subdirectory( exrdisplay )
else()
  if(NOT FLTK_FOUND)
    message(WARNING "FLTK not found, exrdisplay will not be built")
  endif()
  if(NOT OpenGL_FOUND)
    message(WARNING "OpenGL not found, exrdisplay will not be built")
  endif()
endif()

if(NOT WIN32)
  find_package(GLUT)
  # nvidia doesn't seem to provide one of these...
  #find_package(Cg)
  include(config/LocateCg.cmake)

  if(TARGET OpenGL::GLU AND TARGET Cg::CgGL AND TARGET GLUT::GLUT)
    add_subdirectory( playexr )
  endif()
else()
  message(NOTICE ": Disabling playexr, not yet ported to WIN32 platforms")
endif()

#[==[

##########################
# Installation
##########################

# Documentation
INSTALL ( FILES
  doc/OpenEXRViewers.pdf
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}/share/doc/OpenEXR-${OPENEXR_VERSION}
  )

#]==]

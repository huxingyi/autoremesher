# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[

  CMake Configuration for OpenVDB Maya

#]=======================================================================]

cmake_minimum_required(VERSION 3.3)
project(OpenVDBMaya)

# Monitoring <PackageName>_ROOT variables
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

###### OpenVDB Maya Options

option(OPENVDB_MAYA_INSTALL_MOD "Install module file" ON)

if(OPENVDB_MAYA_SUBDIR)
  message(DEPRECATION "OPENVDB_MAYA_SUBDIR is deprecated and will be removed. "
    "Set OPENVDB_MAYA_INSTALL_PREFIX to your install location."
  )
endif()

##########################################################################

message(STATUS "----------------------------------------------------" )
message(STATUS "------------- Configuring OpenVDBMaya --------------" )
message(STATUS "----------------------------------------------------" )

#########################################################################

# Configure prefix

if(NOT OPENVDB_MAYA_INSTALL_PREFIX)
  if(OPENVDB_MAYA_SUBDIR)
    set(MAYA_INSTALL_LOCATION maya/${Maya_VERSION})
  else()
    set(MAYA_INSTALL_LOCATION maya${Maya_VERSION})
  endif()
endif()

set(OPENVDB_MAYA_INSTALL_PREFIX ${MAYA_INSTALL_LOCATION}
  CACHE PATH "Base install path for OpenVDB Maya libraries and headers."
)
set(OPENVDB_MAYA_PLUGINS ${OPENVDB_MAYA_INSTALL_PREFIX}/plug-ins
  CACHE PATH "Install path for OpenVDB Maya plugins"
)
set(OPENVDB_MAYA_SCRIPTS ${OPENVDB_MAYA_INSTALL_PREFIX}/scripts
  CACHE PATH "Install path for OpenVDB Maya scripts"
)

message(STATUS "Maya base install path: ${OPENVDB_MAYA_INSTALL_PREFIX}")

#########################################################################

# Collect and configure lib dependencies

if(NOT OPENVDB_BUILD_CORE)
  set(OPENVDB_LIB OpenVDB::openvdb)
else()
  set(OPENVDB_LIB openvdb)
endif()

find_package(OpenGL REQUIRED)

set(OPENVDB_MAYA_DEPENDENT_LIBS
  ${OPENVDB_LIB}
  Maya
  OpenGL::GL
)

#########################################################################

# WARNING:  Compilation order of these files is extremely important. On
# MacOS/Clang (and potentially other platform/compiler combos), TBB fails
# to mutex lock the node registry in OpenVDBPlugin.cc if OpenVDBPlugin.cc
# is linked in after any of the nodes which create NodeRegistry objects.
# This is most likely due to the construction order of file static data.
# @todo  Restructure the node registry implementation

set(OPENVDB_MAYA_MODULE_SOURCE_FILES
  OpenVDBData.cc
  OpenVDBPlugin.cc
  OpenVDBCopyNode.cc
  OpenVDBFilterNode.cc
  OpenVDBFromMayaFluidNode.cc
  OpenVDBFromPolygonsNode.cc
  OpenVDBReadNode.cc
  OpenVDBToPolygonsNode.cc
  OpenVDBTransformNode.cc
  OpenVDBUtil.cc
  OpenVDBVisualizeNode.cc
  OpenVDBWriteNode.cc
)

set(OPENVDB_MAYA_MODULE_MEL_FILES
  AEOpenVDBCopyTemplate.mel
  AEOpenVDBFilterTemplate.mel
  AEOpenVDBFromMayaFluidTemplate.mel
  AEOpenVDBFromPolygonsTemplate.mel
  AEOpenVDBReadTemplate.mel
  AEOpenVDBToPolygonsTemplate.mel
  AEOpenVDBTransformTemplate.mel
  AEOpenVDBVisualizeTemplate.mel
  AEOpenVDBWriteTemplate.mel
)

#########################################################################

# This is to work around DWA way of doing stuff
set(OPENVDB_MAYA_HEADERS
  "OpenVDBData.h"
  "OpenVDBUtil.h"
)

set(OPENVDB_MAYA_DIR ${PROJECT_BINARY_DIR}/openvdb_maya)
file(MAKE_DIRECTORY ${OPENVDB_MAYA_DIR})
file(COPY ${OPENVDB_MAYA_HEADERS} DESTINATION ${OPENVDB_MAYA_DIR})

#########################################################################

add_library(openvdb_maya SHARED
  ${OPENVDB_MAYA_MODULE_SOURCE_FILES}
)

target_include_directories(openvdb_maya PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(openvdb_maya ${OPENVDB_MAYA_DEPENDENT_LIBS})

maya_set_library_properties(openvdb_maya)

set_target_properties(openvdb_maya
  PROPERTIES
  OUTPUT_NAME OpenVDB
)

if(OPENVDB_ENABLE_RPATH)
  set(RPATHS "")
  if(OPENVDB_BUILD_CORE)
    list(APPEND RPATHS "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
  else()
    list(APPEND RPATHS ${OpenVDB_LIBRARY_DIRS})
  endif()

  list(REMOVE_DUPLICATES RPATHS)
  set_target_properties(openvdb_maya
    PROPERTIES INSTALL_RPATH "${RPATHS}"
  )
  unset(RPATHS)
endif()

#########################################################################

install(TARGETS openvdb_maya
  DESTINATION ${OPENVDB_MAYA_PLUGINS}
)

install(FILES ${OPENVDB_MAYA_MODULE_MEL_FILES}
  DESTINATION ${OPENVDB_MAYA_SCRIPTS}
)

if(OPENVDB_MAYA_INSTALL_MOD)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/openvdb.mod
    "+ openvdb any ${OPENVDB_MAYA_INSTALL_PREFIX}\n")
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/openvdb.mod
    DESTINATION ${OPENVDB_MAYA_INSTALL_PREFIX}
    COMPONENT Runtime
  )
endif()

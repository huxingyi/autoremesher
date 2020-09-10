# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

set(Cg_ROOT "/usr" CACHE STRING "Override location for Cg libraries")
find_path(Cg_INCLUDE_DIR NAMES cgGL.h
  PATHS /usr/include /usr/local/include ${Cg_ROOT}/include
  PATH_SUFFIXES Cg
)
if(Cg_INCLUDE_DIR)
  find_library(Cg_LIBRARY Cg PATHS /usr/lib /usr/lib64 ${Cg_ROOT}/lib)
  if (Cg_LIBRARY)
    add_library(Cg UNKNOWN IMPORTED GLOBAL)
    set_target_properties(Cg PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${Cg_INCLUDE_DIR}
      IMPORTED_LOCATION ${Cg_LIBRARY}
    )
    message(NOTICE "-- Found Cg library")
    add_library(Cg::Cg ALIAS Cg)
  else()
    message(WARNING "Unable to locate Cg library")
  endif()
  find_library(CgGL_LIBRARY CgGL PATHS /usr/lib /usr/lib64 ${Cg_ROOT}/lib)
  if (CgGL_LIBRARY)
    add_library(CgGL UNKNOWN IMPORTED GLOBAL)
    set_target_properties(CgGL PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${Cg_INCLUDE_DIR}
      INTERFACE_LINK_LIBRARIES Cg
      IMPORTED_LOCATION ${CgGL_LIBRARY}
    )
    add_library(Cg::CgGL ALIAS CgGL)
    message(NOTICE "-- Found CgGL library")
  else()
    message(WARNING "Unable to locate CgGL library")
  endif()
else()
  set(Cg-NOTFOUND TRUE)
  message(WARNING "Unable to locate Cg header")
endif()

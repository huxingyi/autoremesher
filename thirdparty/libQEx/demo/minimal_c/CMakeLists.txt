#
# Copyright 2013 Computer Graphics Group, RWTH Aachen University
# Author: Hans-Christian Ebke <ebke@cs.rwth-aachen.de>
#
# This file is part of QEx.
# 
# QEx is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your
# option) any later version.
# 
# QEx is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with QEx.  If not, see <http://www.gnu.org/licenses/>.
#

find_package (OpenMesh REQUIRED)

set (DEMO_MINIMAL_C_SOURCES "")
set (DEMO_MINIMAL_C_INCLUDE_DIRS "")
set (DEMO_MINIMAL_C_LIBRARIES "")
set (DEMO_MINIMAL_C_LIBRARY_DIRS "")

list (APPEND DEMO_MINIMAL_C_SOURCES main.c)
list (APPEND DEMO_MINIMAL_C_INCLUDE_DIRS
    ${OPENMESH_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../interfaces/c
    ${QEX_INCLUDE_DIRS}
    )
list (APPEND DEMO_MINIMAL_C_LIBRARIES
    ${OPENMESH_LIBRARIES}
    QEx
    )
list (APPEND DEMO_MINIMAL_C_LIBRARY_DIRS ${OPEMESH_LIBRARY_DIR})

if (NOT WIN32 )

  add_executable (demo_minimal_c ${DEMO_MINIMAL_C_SOURCES})

  target_link_libraries(demo_minimal_c ${DEMO_MINIMAL_C_LIBRARIES})

  set_target_properties(demo_minimal_c
      PROPERTIES
      INCLUDE_DIRECTORIES "${DEMO_MINIMAL_C_INCLUDE_DIRS}"
      COMPILE_FLAGS "-Wall -Wextra -pedantic -std=c99"
  )

endif()

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

set (CMDLINE_TOOL_SOURCES "")
set (CMDLINE_TOOL_INCLUDE_DIRS "")
set (CMDLINE_TOOL_LIBRARIES "")
set (CMDLINE_TOOL_LIBRARY_DIRS "")

list (APPEND CMDLINE_TOOL_SOURCES main.cpp)
list (APPEND CMDLINE_TOOL_INCLUDE_DIRS
    ${OPENMESH_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../interfaces/c
    ${QEX_INCLUDE_DIRS}
)
list (APPEND CMDLINE_TOOL_LIBRARIES
    ${OPENMESH_LIBRARIES}
    QEx
)
list (APPEND CMDLINE_TOOL_LIBRARY_DIRS ${OPEMESH_LIBRARY_DIR})





if (NOT WIN32 )

  add_executable (cmdline_tool ${CMDLINE_TOOL_SOURCES})

  target_link_libraries(cmdline_tool ${CMDLINE_TOOL_LIBRARIES})

  set_target_properties(cmdline_tool
      PROPERTIES
      INCLUDE_DIRECTORIES "${CMDLINE_TOOL_INCLUDE_DIRS}"
      COMPILE_FLAGS "-Wall -Wextra -pedantic -std=c++98 -Wno-long-long"
  )

endif()


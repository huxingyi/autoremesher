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

cmake_minimum_required(VERSION 2.6)
project(QEx_UnitTests)

if (NOT WIN32)
  add_definitions(-Wall -Wextra -Wno-unused-parameter)
endif()

FIND_PACKAGE(Threads)
enable_testing()

file(GLOB_RECURSE TEST_SOURCES *.cc)
file(GLOB TEST_MESHES meshes/*.obj)

file(COPY ${TEST_MESHES} DESTINATION ${CMAKE_BINARY_DIR}/meshes)

include_directories("${GTEST_DIR}/include" ${OPENMESH_INCLUDE_DIR} "../interfaces/c/")
add_executable(QEx_tests ${TEST_SOURCES})
target_link_libraries(QEx_tests
    gtest QEx ${OPENMESH_LIBRARY} ${CMAKE_THREAD_LIBS_INIT}
    )
add_test(AllTestsIn_QEx_tests QEx_tests)

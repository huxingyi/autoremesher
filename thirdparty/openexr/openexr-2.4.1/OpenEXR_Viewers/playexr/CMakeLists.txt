# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the OpenEXR Project.

add_executable(playexr
  main.cpp 
  playExr.cpp
  playExr.h
  fileNameForFrame.cpp
  fileNameForFrame.h
  FileReadingThread.cpp
  FileReadingThread.h
  ImageBuffers.cpp
  ImageBuffers.h
  Timer.cpp
  Timer.h
  ctlToLut.cpp
  ctlToLut.h
  osDependent.cpp
  osDependent.h 
)

target_link_libraries(playexr
  Cg::CgGL
  GLUT::GLUT
  OpenGL::GL
  OpenGL::GLU
  OpenEXR::IlmImf
)
set_target_properties(playexr PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS playexr DESTINATION ${CMAKE_INSTALL_BINDIR})

include (ACGCommon)

include_directories (
  ../../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set (headers
  VDPMSynthesizerViewerWidget.hh
)

set (sources
  ../../QtViewer/QGLViewerWidget.cc
  vdpmsynthesizer.cc
  VDPMSynthesizerViewerWidget.cc
)


if (WIN32)
  acg_add_executable (Synthesizer WIN32 ${sources} ${headers})
else ()
  acg_add_executable (Synthesizer ${sources} ${headers})
endif ()

target_link_libraries (Synthesizer
  OpenMeshCore
  OpenMeshTools
  Qt5::OpenGL
  ${OPENGL_LIBRARIES}
)

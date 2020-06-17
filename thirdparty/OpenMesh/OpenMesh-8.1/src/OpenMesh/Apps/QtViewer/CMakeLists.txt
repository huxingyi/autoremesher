include (ACGCommon)

include_directories (
  ../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# source code directories
set (directories 
  .
)

set (sources
  MeshViewerWidget.cc
  QGLViewerWidget.cc
  meshviewer.cc
)

set (headers
  MeshViewerWidget.hh
  QGLViewerWidget.hh  
)

if (WIN32)
  acg_add_executable (QtViewer WIN32 ${sources} ${headers})
else ()
  acg_add_executable (QtViewer ${sources} ${headers})
endif ()

target_link_libraries (QtViewer
  OpenMeshCore
  OpenMeshTools
  Qt5::OpenGL
  ${OPENGL_LIBRARIES}
)


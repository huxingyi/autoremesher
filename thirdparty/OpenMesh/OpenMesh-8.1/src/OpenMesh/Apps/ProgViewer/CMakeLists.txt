include (ACGCommon)

include_directories (
  ../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)


set( headers
  ProgViewerWidget.hh
  ../QtViewer/QGLViewerWidget.hh
  ../QtViewer/MeshViewerWidgetT.hh
  ../QtViewer/MeshViewerWidgetT_impl.hh
)

set( sources
  ProgViewerWidget.cc
  progviewer.cc  
  ../QtViewer/QGLViewerWidget.cc
)

if (WIN32)
  acg_add_executable( ProgViewer WIN32 ${sources} ${headers})
else ()
  acg_add_executable( ProgViewer ${sources} ${headers})
endif ()

target_link_libraries ( ProgViewer
  OpenMeshCore
  OpenMeshTools
  Qt5::OpenGL
  ${OPENGL_LIBRARIES}
)


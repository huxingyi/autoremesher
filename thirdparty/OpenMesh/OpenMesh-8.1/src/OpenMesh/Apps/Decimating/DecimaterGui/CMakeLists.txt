include (ACGCommon)

include_directories (
  ../../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set (headers
  ../DecimaterViewerWidget.hh
  ../../QtViewer/QGLViewerWidget.hh
  ../../QtViewer/MeshViewerWidgetT.hh
  ../../QtViewer/MeshViewerWidget.hh
  ../../QtViewer/MeshViewerWidgetT_impl.hh
)

set (sources
  ../../QtViewer/QGLViewerWidget.cc
  ../../QtViewer/MeshViewerWidget.cc
  ../DecimaterViewerWidget.cc
  ../decimaterviewer.cc
)

if (WIN32)
  acg_add_executable (DecimaterGui WIN32 ${sources} ${headers})
else ()
  acg_add_executable (DecimaterGui ${sources} ${headers} ) 
endif ()

target_link_libraries (DecimaterGui
  OpenMeshCore
  OpenMeshTools
  Qt5::OpenGL
  ${OPENGL_LIBRARIES}
)


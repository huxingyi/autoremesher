include (ACGCommon)

include_directories (
  ../../../..
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set (headers
  ../MeshViewerWidget.hh
  ../SubdivideWidget.hh
  ../../QtViewer/QGLViewerWidget.hh
  ../../QtViewer/MeshViewerWidgetT.hh
  ../../QtViewer/MeshViewerWidgetT_impl.hh
)

set (sources
  ../../QtViewer/QGLViewerWidget.cc
  ../SubdivideWidget.cc
  ../qtsubdivider.cc
)

if (WIN32)
  acg_add_executable (SubdividerGui WIN32 ${sources} ${headers})
else ()
  acg_add_executable (SubdividerGui ${sources} ${headers})
endif ()

target_link_libraries (SubdividerGui
  OpenMeshCore
  OpenMeshTools
  Qt5::OpenGL
  ${OPENGL_LIBRARIES}
)


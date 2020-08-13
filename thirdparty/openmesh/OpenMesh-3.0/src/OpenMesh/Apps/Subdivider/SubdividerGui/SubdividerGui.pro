################################################################################
#
################################################################################

include( $$TOPDIR/qmake/all.include )

INCLUDEPATH += ../../..

Application()
glew()
glut()
openmesh()

DIRECTORIES = .. ../../QtViewer

# Input
HEADERS += $$getFilesFromDir($$DIRECTORIES,*.hh)
SOURCES += ../../QtViewer/QGLViewerWidget.cc ../../QtViewer/MeshViewerWidgetT.cc ../SubdivideWidget.cc
SOURCES += ../qtsubdivider.cc

################################################################################

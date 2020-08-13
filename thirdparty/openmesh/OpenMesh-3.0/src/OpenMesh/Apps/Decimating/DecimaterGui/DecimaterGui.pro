################################################################################
#
################################################################################

include( $$TOPDIR/qmake/all.include )

Application()

INCLUDEPATH += ../../..

Application()
glew()
glut()
openmesh()

DIRECTORIES = ../../QtViewer ../

# Input
HEADERS += $$getFilesFromDir($$DIRECTORIES,*.hh)
SOURCES += ../../QtViewer/QGLViewerWidget.cc ../../QtViewer/MeshViewerWidgetT.cc ../DecimaterViewerWidget.cc
SOURCES += ../decimaterviewer.cc


################################################################################

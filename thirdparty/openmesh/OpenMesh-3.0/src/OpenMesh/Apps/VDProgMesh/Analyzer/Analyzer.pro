################################################################################
#
################################################################################

include( $$TOPDIR/qmake/all.include )

INCLUDEPATH += ../../../..

CONFIG += glew glut

Application()

LIBS         += -Wl,-rpath=$${TOPDIR}/OpenMesh/Core/lib/$${BUILDDIRECTORY} -lCore
LIBS         += -Wl,-rpath=$${TOPDIR}/OpenMesh/Tools/lib/$${BUILDDIRECTORY} -lTools
LIBS 	     += -lglut
QMAKE_LIBDIR += $${TOPDIR}/OpenMesh/Core/lib/$${BUILDDIRECTORY}
QMAKE_LIBDIR += $${TOPDIR}/OpenMesh/Tools/lib/$${BUILDDIRECTORY}

DIRECTORIES = .

# Input
HEADERS += $$getFilesFromDir($$DIRECTORIES,*.hh)
SOURCES += $$getFilesFromDir($$DIRECTORIES,*.cc)
FORMS   += $$getFilesFromDir($$DIRECTORIES,*.ui)

################################################################################

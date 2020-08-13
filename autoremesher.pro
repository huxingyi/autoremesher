QT += core widgets opengl network
CONFIG += release
#DEFINES += AUTO_REMESHER_DEV
CONFIG(release, debug|release) DEFINES += NDEBUG
DEFINES += AUTO_REMESHER_DEBUG
DEFINES += QT_MESSAGELOGCONTEXT
RESOURCES += resources.qrc

#dumpbin release\autoremesher.exe /DEPENDENTS

CONFIG(debug, debug|release) OBJECTS_DIR=obj-debug
CONFIG(release, debug|release) OBJECTS_DIR=obj
CONFIG(debug, debug|release) MOC_DIR=moc-debug
CONFIG(release, debug|release) MOC_DIR=moc

win32 {
    CONFIG(debug, debug|release) CONFIG += force_debug_info
	RC_FILE = autoremesher.rc
}

macx {
	ICON = autoremesher.icns

	RESOURCE_FILES.files = $$ICON
	RESOURCE_FILES.path = Contents/Resources
	QMAKE_BUNDLE_DATA += RESOURCE_FILES
}

isEmpty(HUMAN_VERSION) {
	HUMAN_VERSION = "1.0.0-alpha.4"
}
isEmpty(VERSION) {
	VERSION = 1.0.0.4
}

HOMEPAGE_URL = "https://autoremesher.dust3d.org/"
REPOSITORY_URL = "https://github.com/huxingyi/autoremesher"
ISSUES_URL = "https://github.com/huxingyi/autoremesher/issues"
UPDATES_CHECKER_URL = "https://dust3d.org/autoremesher-updateinfo.xml"

PLATFORM = "Unknown"
macx {
	PLATFORM = "MacOS"
}
win32 {
	PLATFORM = "Win32"
}
unix:!macx {
	PLATFORM = "Linux"
}

QMAKE_TARGET_COMPANY = Dust3D
QMAKE_TARGET_PRODUCT = AutoRemesher
QMAKE_TARGET_DESCRIPTION = "AutoRemesher is a cross-platform open-source automatic quad remeshing software"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2020 AutoRemesher Project. All Rights Reserved."

DEFINES += "PROJECT_DEFINED_APP_COMPANY=\"\\\"$$QMAKE_TARGET_COMPANY\\\"\""
DEFINES += "PROJECT_DEFINED_APP_NAME=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\""
DEFINES += "PROJECT_DEFINED_APP_VER=\"\\\"$$VERSION\\\"\""
DEFINES += "PROJECT_DEFINED_APP_HUMAN_VER=\"\\\"$$HUMAN_VERSION\\\"\""
DEFINES += "PROJECT_DEFINED_APP_HOMEPAGE_URL=\"\\\"$$HOMEPAGE_URL\\\"\""
DEFINES += "PROJECT_DEFINED_APP_REPOSITORY_URL=\"\\\"$$REPOSITORY_URL\\\"\""
DEFINES += "PROJECT_DEFINED_APP_ISSUES_URL=\"\\\"$$ISSUES_URL\\\"\""
DEFINES += "PROJECT_DEFINED_APP_UPDATES_CHECKER_URL=\"\\\"$$UPDATES_CHECKER_URL\\\"\""
DEFINES += "PROJECT_DEFINED_APP_PLATFORM=\"\\\"$$PLATFORM\\\"\""

CONFIG += c++14

macx {
	QMAKE_CXXFLAGS_RELEASE -= -O
	QMAKE_CXXFLAGS_RELEASE -= -O1
	QMAKE_CXXFLAGS_RELEASE -= -O2

	QMAKE_CXXFLAGS_RELEASE += -O3
}

unix:!macx {
	QMAKE_CXXFLAGS_RELEASE -= -O
	QMAKE_CXXFLAGS_RELEASE -= -O1
	QMAKE_CXXFLAGS_RELEASE -= -O2

	QMAKE_CXXFLAGS_RELEASE += -O3
}

win32 {
	CONFIG(debug, debug|release) QMAKE_CXXFLAGS += /Od
    CONFIG(release, debug|release) QMAKE_CXXFLAGS += /O2
	QMAKE_CXXFLAGS += /bigobj
}

DEFINES += _USE_MATH_DEFINES

include(thirdparty/QtAwesome/QtAwesome/QtAwesome.pri)

INCLUDEPATH += thirdparty/QtWaitingSpinner

SOURCES += thirdparty/QtWaitingSpinner/waitingspinnerwidget.cpp
HEADERS += thirdparty/QtWaitingSpinner/waitingspinnerwidget.h

INCLUDEPATH += include

SOURCES += src/main.cpp

SOURCES += src/logbrowser.cpp
HEADERS += src/logbrowser.h

SOURCES += src/logbrowserdialog.cpp
HEADERS += src/logbrowserdialog.h

SOURCES += src/spinnableawesomebutton.cpp
HEADERS += src/spinnableawesomebutton.h

SOURCES += src/util.cpp
HEADERS += src/util.h

SOURCES += src/updateschecker.cpp
HEADERS += src/updateschecker.h

SOURCES += src/mainwindow.cpp
HEADERS += src/mainwindow.h

SOURCES += src/aboutwidget.cpp
HEADERS += src/aboutwidget.h

SOURCES += src/theme.cpp
HEADERS += src/theme.h

SOURCES += src/graphicscontainerwidget.cpp
HEADERS += src/graphicscontainerwidget.h

SOURCES += src/graphicswidget.cpp
HEADERS += src/graphicswidget.h

SOURCES += src/updatescheckwidget.cpp
HEADERS += src/updatescheckwidget.h

SOURCES += src/pbrshadermesh.cpp
HEADERS += src/pbrshadermesh.h

SOURCES += src/pbrshadermeshbinder.cpp
HEADERS += src/pbrshadermeshbinder.h

SOURCES += src/pbrshaderprogram.cpp
HEADERS += src/pbrshaderprogram.h

HEADERS += src/pbrshadervertex.h

SOURCES += src/pbrshaderwidget.cpp
HEADERS += src/pbrshaderwidget.h

SOURCES += src/rendermeshgenerator.cpp
HEADERS += src/rendermeshgenerator.h

SOURCES += src/quadmeshgenerator.cpp
HEADERS += src/quadmeshgenerator.h

SOURCES += src/ddsfile.cpp
HEADERS += src/ddsfile.h

SOURCES += src/preferences.cpp
HEADERS += src/preferences.h

SOURCES += src/AutoRemesher/autoremesher.cpp
HEADERS += src/AutoRemesher/autoremesher.h

SOURCES += src/AutoRemesher/quadremesher.cpp
HEADERS += src/AutoRemesher/quadremesher.h

SOURCES += src/AutoRemesher/isotropicremesher.cpp
HEADERS += src/AutoRemesher/isotropicremesher.h

SOURCES += src/AutoRemesher/meshcutter.cpp
HEADERS += src/AutoRemesher/meshcutter.h

SOURCES += src/AutoRemesher/halfedge.cpp
HEADERS += src/AutoRemesher/halfedge.h

SOURCES += src/AutoRemesher/parameterizer.cpp
HEADERS += src/AutoRemesher/parameterizer.h

INCLUDEPATH += thirdparty/tbb/include
unix {
	LIBS += -Lthirdparty/tbb/build2 -ltbbmalloc_proxy_static -ltbbmalloc_static -ltbb_static
	unix:!macx {
		LIBS += -ldl
	}
}
win32 {
	CONFIG(debug, debug|release) LIBS += -Lthirdparty/tbb/build2-debug/Debug -ltbb_static_debug -ltbbmalloc_static_debug -ltbbmalloc_proxy_static_debug
    CONFIG(release, debug|release) LIBS += -Lthirdparty/tbb/build2/Release -ltbb_static -ltbbmalloc_static -ltbbmalloc_proxy_static
}

INCLUDEPATH += thirdparty/comiso
INCLUDEPATH += thirdparty/comiso/Solver
DEFINES += _SCL_SECURE_NO_DEPRECATE     #for comiso\ext\gmm-4.2\include\gmm\gmm_std.h
DEFINES += INCLUDE_TEMPLATES
INCLUDEPATH += thirdparty/comiso/CoMISo/ext/gmm-4.2/include
INCLUDEPATH += thirdparty/libigl/include
INCLUDEPATH += thirdparty/eigen

SOURCES += thirdparty/comiso/CoMISo/Solver/EigenLDLTSolver.cc
HEADERS += thirdparty/comiso/CoMISo/Solver/EigenLDLTSolver.hh

SOURCES += thirdparty/comiso/CoMISo/Solver/MISolver.cc
HEADERS += thirdparty/comiso/CoMISo/Solver/MISolver.hh

SOURCES += thirdparty/comiso/CoMISo/Solver/ConstrainedSolver.cc
HEADERS += thirdparty/comiso/CoMISo/Solver/ConstrainedSolver.hh

SOURCES += thirdparty/comiso/CoMISo/Solver/IterativeSolverT.cc
HEADERS += thirdparty/comiso/CoMISo/Solver/IterativeSolverT.hh

SOURCES += thirdparty/comiso/CoMISo/Solver/GMM_Tools.cc
HEADERS += thirdparty/comiso/CoMISo/Solver/GMM_Tools.hh

INCLUDEPATH += thirdparty/openmesh/OpenMesh-3.0/src

INCLUDEPATH += thirdparty/libQEx/src

INCLUDEPATH += thirdparty/libQEx/interfaces/c

HEADERS += thirdparty/libQEx/src/Algebra.hh

HEADERS += thirdparty/libQEx/src/ExactPredicates.h

HEADERS += thirdparty/libQEx/src/Globals.hh

HEADERS += thirdparty/libQEx/src/MeshDecimatorT.hh

SOURCES += thirdparty/libQEx/src/MeshExtractor.cc

SOURCES += thirdparty/libQEx/src/MeshExtractorT.cc
HEADERS += thirdparty/libQEx/src/MeshExtractorT.hh

SOURCES += thirdparty/libQEx/src/predicates.c

SOURCES += thirdparty/libQEx/src/QuadExtractorPostprocT.cc
HEADERS += thirdparty/libQEx/src/QuadExtractorPostprocT.hh

HEADERS += thirdparty/libQEx/src/TransitionFunction.hh

HEADERS += thirdparty/libQEx/src/Vector.hh

SOURCES += thirdparty/libQEx/interfaces/c/qex.cc
HEADERS += thirdparty/libQEx/interfaces/c/qex.h

CONFIG(debug, debug|release) LIBS += -Lthirdparty/openmesh/OpenMesh-3.0/build-debug/Build/lib -lOpenMeshCored
CONFIG(release, debug|release) LIBS += -Lthirdparty/openmesh/OpenMesh-3.0/build/Build/lib -lOpenMeshCore

win32 {
    LIBS += -luser32
	LIBS += -lopengl32

	isEmpty(BOOST_INCLUDEDIR) {
		BOOST_INCLUDEDIR = $$(BOOST_INCLUDEDIR)
	}
	isEmpty(CGAL_DIR) {
		CGAL_DIR = $$(CGAL_DIR)
	}

	isEmpty(BOOST_INCLUDEDIR) {
		error("No BOOST_INCLUDEDIR define found in enviroment variables")
	}

	isEmpty(CGAL_DIR) {
		error("No CGAL_DIR define found in enviroment variables")
	}

	GMP_LIBNAME = libgmp-10
	MPFR_LIBNAME = libmpfr-4
	CGAL_INCLUDEDIR = $$CGAL_DIR\include
	GMP_INCLUDEDIR = $$CGAL_DIR\auxiliary\gmp\include
	GMP_LIBDIR = $$CGAL_DIR\auxiliary\gmp\lib
	MPFR_INCLUDEDIR = $$GMP_INCLUDEDIR
	MPFR_LIBDIR = $$GMP_LIBDIR
}

macx {
	GMP_LIBNAME = gmp
	MPFR_LIBNAME = mpfr
	BOOST_INCLUDEDIR = /usr/local/opt/boost/include
	CGAL_INCLUDEDIR = /usr/local/opt/cgal/include
	GMP_INCLUDEDIR = /usr/local/opt/gmp/include
	GMP_LIBDIR = /usr/local/opt/gmp/lib
	MPFR_INCLUDEDIR = /usr/local/opt/mpfr/include
	MPFR_LIBDIR = /usr/local/opt/mpfr/lib
}

unix:!macx {
	GMP_LIBNAME = gmp
	MPFR_LIBNAME = mpfr
	BOOST_INCLUDEDIR = /usr/local/include
	CGAL_INCLUDEDIR = /usr/local/include
	GMP_INCLUDEDIR = /usr/local/include
	GMP_LIBDIR = /usr/local/lib
	MPFR_INCLUDEDIR = /usr/local/include
	MPFR_LIBDIR = /usr/local/lib
}

INCLUDEPATH += $$BOOST_INCLUDEDIR

INCLUDEPATH += $$GMP_INCLUDEDIR
LIBS += -L$$GMP_LIBDIR -l$$GMP_LIBNAME

INCLUDEPATH += $$MPFR_INCLUDEDIR
LIBS += -L$$MPFR_LIBDIR -l$$MPFR_LIBNAME

INCLUDEPATH += $$CGAL_INCLUDEDIR

target.path = ./
INSTALLS += target
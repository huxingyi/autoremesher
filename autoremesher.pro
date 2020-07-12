QT += core
CONFIG += console release
DEFINES += NDEBUG

#dumpbin release\autoremesher.exe /DEPENDENTS

OBJECTS_DIR=obj
MOC_DIR=moc

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

macx {
	QMAKE_CXXFLAGS_RELEASE -= -O
	QMAKE_CXXFLAGS_RELEASE -= -O1
	QMAKE_CXXFLAGS_RELEASE -= -O2

	QMAKE_CXXFLAGS_RELEASE += -O3

	QMAKE_CXXFLAGS += -std=c++11
}

unix:!macx {
	QMAKE_CXXFLAGS_RELEASE -= -O
	QMAKE_CXXFLAGS_RELEASE -= -O1
	QMAKE_CXXFLAGS_RELEASE -= -O2

	QMAKE_CXXFLAGS_RELEASE += -O3

	QMAKE_CXXFLAGS += -std=c++11
}

win32 {
	QMAKE_CXXFLAGS += /O2
	QMAKE_CXXFLAGS += /bigobj
}

DEFINES += _SCL_SECURE_NO_DEPRECATE
DEFINES += _USE_MATH_DEFINES
DEFINES += INCLUDE_TEMPLATES

INCLUDEPATH += include

SOURCES += src/main.cpp

SOURCES += src/AutoRemesher/remesher.cpp
HEADERS += src/AutoRemesher/remesher.h

SOURCES += src/AutoRemesher/halfedge.cpp
HEADERS += src/AutoRemesher/halfedge.h

SOURCES += src/AutoRemesher/parametrization.cpp
HEADERS += src/AutoRemesher/parametrization.h

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

INCLUDEPATH += thirdparty/OpenMesh/OpenMesh-8.1/src

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

LIBS += -Lthirdparty/OpenMesh/OpenMesh-8.1/build/Build/lib -lOpenMeshCore

win32 {
    LIBS += -luser32
}

target.path = ./
INSTALLS += target
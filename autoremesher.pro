QT += core widgets opengl
# QOpenGLWidget moved into its own module in Qt 6.
greaterThan(QT_MAJOR_VERSION, 5): QT += openglwidgets
win32 {
    qtHaveModule(winextras) {
        QT += winextras
    }
}
CONFIG += release
CONFIG(release, debug|release) DEFINES += NDEBUG
CONFIG(debug, debug|release) DEFINES += AUTO_REMESHER_DEBUG
CONFIG(debug, debug|release) DEFINES += QT_MESSAGELOGCONTEXT
RESOURCES += resources.qrc

CONFIG += object_parallel_to_source

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
	HUMAN_VERSION = "1.0.0"
}
isEmpty(VERSION) {
	VERSION = 1.0.0.9
}

HOMEPAGE_URL = "https://autoremesher.dust3d.org/"
REPOSITORY_URL = "https://github.com/huxingyi/autoremesher"
ISSUES_URL = "https://github.com/huxingyi/autoremesher/issues"
PLATFORM = "Unknown"

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
DEFINES += "PROJECT_DEFINED_APP_PLATFORM=\"\\\"$$PLATFORM\\\"\""

CONFIG += c++14

macx {
	QMAKE_CXXFLAGS_RELEASE -= -O
	QMAKE_CXXFLAGS_RELEASE -= -O1
	QMAKE_CXXFLAGS_RELEASE -= -O2

	QMAKE_CXXFLAGS_RELEASE += -O3
	# LTO and aggressive loop optimizations — trade code size for speed
	QMAKE_CXXFLAGS_RELEASE += -flto -funroll-loops
	QMAKE_LFLAGS_RELEASE += -flto

	QMAKE_CFLAGS += -DNL_USE_BLAS
	QMAKE_CXXFLAGS += -DNL_USE_BLAS
	DEFINES += AUTO_REMESHER_USE_ACCELERATE=1
	LIBS += -framework Accelerate
}

unix:!macx {
	QMAKE_CXXFLAGS_RELEASE -= -O
	QMAKE_CXXFLAGS_RELEASE -= -O1
	QMAKE_CXXFLAGS_RELEASE -= -O2

	QMAKE_CXXFLAGS_RELEASE += -O3
	# LTO, aggressive loop unrolling, and x86-64-v2 baseline for broad CPU compatibility
	QMAKE_CXXFLAGS_RELEASE += -flto -funroll-loops -march=x86-64-v2
	QMAKE_LFLAGS_RELEASE += -flto
}

win32 {
	CONFIG(debug, debug|release) QMAKE_CXXFLAGS += /Od
    CONFIG(release, debug|release) QMAKE_CXXFLAGS += /O2 /GL /Qpar /fp:fast
	CONFIG(release, debug|release) QMAKE_LFLAGS += /LTCG
	QMAKE_CXXFLAGS += /bigobj
}

DEFINES += _USE_MATH_DEFINES
DEFINES += NOMINMAX

# Qt's qyieldcpu.h calls the __yield ARM builtin without including
# <arm_acle.h>. Since clang 16, -Wimplicit-function-declaration is an error by
# default, which makes Qt headers fail to compile (seen with Qt 6 + clang on
# Apple Silicon). Downgrade it to a warning so the Qt headers build. Scoped to
# clang/gcc (MSVC in the win32 block ignores these flags).
!win32 {
    QMAKE_CXXFLAGS += -Wno-error=implicit-function-declaration
    QMAKE_CFLAGS += -Wno-error=implicit-function-declaration
}

include(thirdparty/QtAwesome/QtAwesome/QtAwesome.pri)

INCLUDEPATH += thirdparty/QtWaitingSpinner

SOURCES += thirdparty/QtWaitingSpinner/waitingspinnerwidget.cpp
HEADERS += thirdparty/QtWaitingSpinner/waitingspinnerwidget.h

INCLUDEPATH += thirdparty/eigen

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

SOURCES += src/modelshadermesh.cpp
HEADERS += src/modelshadermesh.h

SOURCES += src/modelshadermeshbinder.cpp
HEADERS += src/modelshadermeshbinder.h

SOURCES += src/modelshaderprogram.cpp
HEADERS += src/modelshaderprogram.h

HEADERS += src/modelshadervertex.h

SOURCES += src/monochromeopenglprogram.cpp
HEADERS += src/monochromeopenglprogram.h

SOURCES += src/monochromeopenglobject.cpp
HEADERS += src/monochromeopenglobject.h

HEADERS += src/monochromeopenglvertex.h

SOURCES += src/modelshaderwidget.cpp
HEADERS += src/modelshaderwidget.h

SOURCES += src/rendermeshgenerator.cpp
HEADERS += src/rendermeshgenerator.h

SOURCES += src/previewmeshgenerator.cpp
HEADERS += src/previewmeshgenerator.h

SOURCES += src/quadmeshgenerator.cpp
HEADERS += src/quadmeshgenerator.h

SOURCES += src/preferences.cpp
HEADERS += src/preferences.h

SOURCES += src/floatnumberwidget.cpp
HEADERS += src/floatnumberwidget.h

SOURCES += src/intnumberwidget.cpp
HEADERS += src/intnumberwidget.h

SOURCES += src/AutoRemesher/autoremesher.cpp
HEADERS += src/AutoRemesher/autoremesher.h

SOURCES += src/AutoRemesher/isotropicremesher.cpp
HEADERS += src/AutoRemesher/isotropicremesher.h

INCLUDEPATH += thirdparty/isotropicremesher
SOURCES += thirdparty/isotropicremesher/isotropicremesher.cpp
SOURCES += thirdparty/isotropicremesher/isotropichalfedgemesh.cpp
SOURCES += thirdparty/isotropicremesher/axisalignedboundingboxtree.cpp
HEADERS += thirdparty/isotropicremesher/isotropicremesher.h
HEADERS += thirdparty/isotropicremesher/isotropichalfedgemesh.h
HEADERS += thirdparty/isotropicremesher/axisalignedboundingboxtree.h
HEADERS += thirdparty/isotropicremesher/axisalignedboundingbox.h
HEADERS += thirdparty/isotropicremesher/vector3.h
HEADERS += thirdparty/isotropicremesher/vector2.h
HEADERS += thirdparty/isotropicremesher/double.h

SOURCES += src/AutoRemesher/parameterizer.cpp
HEADERS += src/AutoRemesher/parameterizer.h

SOURCES += src/AutoRemesher/surfacemesh.cpp
HEADERS += src/AutoRemesher/surfacemesh.h
HEADERS += include/AutoRemesher/SurfaceMesh
SOURCES += src/AutoRemesher/nativesingularitysimplifier.cpp
HEADERS += src/AutoRemesher/nativesingularitysimplifier.h
HEADERS += include/AutoRemesher/NativeSingularitySimplifier
SOURCES += src/AutoRemesher/constrainedleastsquares.cpp
HEADERS += src/AutoRemesher/constrainedleastsquares.h
HEADERS += include/AutoRemesher/ConstrainedLeastSquares
SOURCES += src/AutoRemesher/mixedintegerleastsquares.cpp
HEADERS += src/AutoRemesher/mixedintegerleastsquares.h
HEADERS += include/AutoRemesher/MixedIntegerLeastSquares
SOURCES += src/AutoRemesher/nativeframefield.cpp
HEADERS += src/AutoRemesher/nativeframefield.h
HEADERS += include/AutoRemesher/NativeFrameField
SOURCES += src/AutoRemesher/nativequadparameterizer.cpp
HEADERS += src/AutoRemesher/nativequadparameterizer.h
HEADERS += include/AutoRemesher/NativeQuadParameterizer


SOURCES += src/AutoRemesher/quadextractor.cpp
HEADERS += src/AutoRemesher/quadextractor.h


SOURCES += src/AutoRemesher/positionkey.cpp
HEADERS += src/AutoRemesher/positionkey.h

SOURCES += src/AutoRemesher/meshseparator.cpp
HEADERS += src/AutoRemesher/meshseparator.h

unix {
    LIBS += -lz
}

macx {
    INCLUDEPATH += /opt/homebrew/opt/tbb/include
    LIBS += -L/opt/homebrew/opt/tbb/lib -ltbbmalloc_proxy -ltbbmalloc -ltbb
}
unix:!macx {
    LIBS += -ltbb -lz -ldl
}
win32 {
    INCLUDEPATH += thirdparty/tbb/include
    CONFIG(release, debug|release) LIBS += -Lthirdparty/tbb/build2/Release -ltbb
}

win32 {
    LIBS += -luser32
	LIBS += -lopengl32
}

target.path = ./
INSTALLS += target

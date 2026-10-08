QT += core
CONFIG += console c++14
CONFIG -= app_bundle

CONFIG(release, debug|release) DEFINES += NDEBUG
CONFIG(debug, debug|release) DEFINES += AUTO_REMESHER_DEBUG

INCLUDEPATH += ../include ../src ../thirdparty/eigen

HEADERS += ../src/meshio.h
SOURCES += ../src/meshio.cpp \
           test_main.cpp

SOURCES += ../src/AutoRemesher/autoremesher.cpp \
           ../src/AutoRemesher/surfacemesh.cpp \
           ../src/AutoRemesher/isotropicremesher.cpp \
           ../src/AutoRemesher/decimator.cpp \
           ../src/AutoRemesher/parameterizer.cpp \
           ../src/AutoRemesher/surfaceparameterizer.cpp \
           ../src/AutoRemesher/quadextractor.cpp \
           ../src/AutoRemesher/quadparameterizer.cpp \
           ../src/AutoRemesher/mixedintegerleastsquares.cpp \
           ../src/AutoRemesher/framefield.cpp \
           ../src/AutoRemesher/positionkey.cpp \
           ../src/AutoRemesher/meshseparator.cpp \
           ../src/quadmeshgenerator.cpp

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

win32-msvc* {
    INCLUDEPATH += ../thirdparty/tbb/include
    CONFIG(release, debug|release) LIBS += -L../thirdparty/tbb/build2/Release -ltbb
}

win32-g++ {
    LIBS += -ltbb12
}

TARGET = autoremesher_tests

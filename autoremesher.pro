QT += core widgets opengl network
# QOpenGLWidget moved into its own module in Qt 6.
greaterThan(QT_MAJOR_VERSION, 5): QT += openglwidgets
win32 {
    qtHaveModule(winextras) {
        QT += winextras
    }
}
CONFIG += release
CONFIG(release, debug|release) DEFINES += NDEBUG
DEFINES += AUTO_REMESHER_DEBUG
DEFINES += QT_MESSAGELOGCONTEXT
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

SOURCES += src/quadmeshgenerator.cpp
HEADERS += src/quadmeshgenerator.h

SOURCES += src/ddsfile.cpp
HEADERS += src/ddsfile.h

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

SOURCES += src/AutoRemesher/quadextractor.cpp
HEADERS += src/AutoRemesher/quadextractor.h

SOURCES += src/AutoRemesher/positionkey.cpp
HEADERS += src/AutoRemesher/positionkey.h

SOURCES += src/AutoRemesher/meshseparator.cpp
HEADERS += src/AutoRemesher/meshseparator.h

unix {
    LIBS += -lz
}

INCLUDEPATH += thirdparty/geogram/geogram-1.8.3/src/lib
INCLUDEPATH += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/libMeshb/sources
INCLUDEPATH += thirdparty/geogram
win32 {
    DEFINES -= UNICODE
    LIBS += -ladvapi32 -lshell32
}

SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/algorithm.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/algorithm.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/android_wrapper.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/argused.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/assert.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/assert.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/atomics.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/attributes.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/attributes.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/b_stream.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/b_stream.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/command_line.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/command_line.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/command_line_args.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/command_line_args.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/common.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/common.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/counted.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/counted.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/environment.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/environment.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/factory.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/factory.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/file_system.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/file_system.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/geofile.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/geofile.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/geometry.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/geometry.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/geometry_nd.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/line_stream.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/line_stream.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/logger.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/logger.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/matrix.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/memory.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/numeric.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/numeric.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/packed_arrays.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/packed_arrays.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/permutation.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/process.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/process.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/process_private.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/process_unix.cpp
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/process_win.cpp
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/progress.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/progress.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/psm.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/quaternion.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/quaternion.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/range.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/smart_pointer.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/stopwatch.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/stopwatch.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/string.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/string.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/thread_sync.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/basic/vecg.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/bibliography/bibliography.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/bibliography/bibliography.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/bibliography/embedded_references.cpp
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/LFS.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/LFS.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/cavity.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay_2d.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay_2d.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay_3d.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay_3d.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay_nn.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay_nn.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay_tetgen.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay_tetgen.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay_triangle.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/delaunay_triangle.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/parallel_delaunay_3d.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/parallel_delaunay_3d.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/periodic.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/periodic.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/periodic_delaunay_3d.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/delaunay/periodic_delaunay_3d.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/color.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/colormap.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/colormap.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_library.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_library.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_rasterizer.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_rasterizer.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_serializer.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_serializer.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_serializer_pgm.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_serializer_pgm.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_serializer_stb.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_serializer_stb.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_serializer_xpm.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/image_serializer_xpm.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/morpho_math.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/image/morpho_math.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_AABB.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_AABB.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_fill_holes.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_fill_holes.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_frame_field.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_frame_field.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_geometry.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_geometry.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_halfedges.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_halfedges.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_io.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_io.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_partition.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_partition.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_preprocessing.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_preprocessing.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_reorder.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_reorder.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_repair.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_repair.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_topology.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/mesh/mesh_topology.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/expansion_nt.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/expansion_nt.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/lbfgs_optimizers.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/lbfgs_optimizers.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/matrix_util.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/matrix_util.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/multi_precision.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/multi_precision.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/optimizer.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/optimizer.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/predicates.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/numerics/predicates.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/parameterization/mesh_global_param.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/parameterization/mesh_global_param.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/points/co3ne.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/points/co3ne.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/points/colocate.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/points/colocate.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/points/kd_tree.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/points/kd_tree.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/points/nn_search.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/points/nn_search.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/points/principal_axes.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/points/principal_axes.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/CVT.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/CVT.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/RVD.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/RVD.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/RVD_callback.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/RVD_callback.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/RVD_mesh_builder.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/RVD_mesh_builder.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/convex_cell.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/convex_cell.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/generic_RVD.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/generic_RVD_cell.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/generic_RVD_cell.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/generic_RVD_polygon.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/generic_RVD_polygon.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/generic_RVD_utils.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/generic_RVD_vertex.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/integration_simplex.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/voronoi/integration_simplex.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/basic/common.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/hexdom/basic.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/hexdom/basic.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/hexdom/frame.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/hexdom/frame.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/hexdom/spherical_harmonics_l4.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/hexdom/spherical_harmonics_l4.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/hexdom/polygon.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/hexdom/polygon.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/hexdom/quad_cover.cpp
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/hexdom/quad_cover.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/exploragram/hexdom/mixed_constrained_solver.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/NL/nl_api.c
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/NL/nl_matrix.c
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/NL/nl_context.c
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/NL/nl_blas.c
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/NL/nl_arpack.c
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/NL/nl_iterative_solvers.c
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/NL/nl_preconditioners.c
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/NL/nl_os.c
SOURCES += src/AutoRemesher/nl_ext_stubs.c
INCLUDEPATH += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/NL
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/libMeshb/sources/libmeshb7.c
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/libMeshb/sources/libmeshb7.h
SOURCES += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/rply/rply.c
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/rply/rply.h
HEADERS += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/rply/rplyfile.h

win32 {
	INCLUDEPATH += thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib
	SOURCES += \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/adler32.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/compress.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/crc32.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/deflate.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/gzclose.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/gzlib.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/gzread.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/gzwrite.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/inffast.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/inflate.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/inftrees.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/trees.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/uncompr.c \
		thirdparty/geogram/geogram-1.8.3/src/lib/geogram/third_party/zlib/zutil.c
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
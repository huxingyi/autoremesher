QT += core widgets opengl network
win32 {
    QT += winextras
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
	HUMAN_VERSION = "1.0.0-beta.3"
}
isEmpty(VERSION) {
	VERSION = 1.0.0.8
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

include(thirdparty/QtAwesome/QtAwesome/QtAwesome.pri)

INCLUDEPATH += thirdparty/QtWaitingSpinner

SOURCES += thirdparty/QtWaitingSpinner/waitingspinnerwidget.cpp
HEADERS += thirdparty/QtWaitingSpinner/waitingspinnerwidget.h

INCLUDEPATH += thirdparty/libigl/include
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

SOURCES += src/floatnumberwidget.cpp
HEADERS += src/floatnumberwidget.h

SOURCES += src/AutoRemesher/autoremesher.cpp
HEADERS += src/AutoRemesher/autoremesher.h

SOURCES += src/AutoRemesher/isotropicremesher.cpp
HEADERS += src/AutoRemesher/isotropicremesher.h

SOURCES += src/AutoRemesher/vdbremesher.cpp
HEADERS += src/AutoRemesher/vdbremesher.h

SOURCES += src/AutoRemesher/parameterizer.cpp
HEADERS += src/AutoRemesher/parameterizer.h

SOURCES += src/AutoRemesher/quadextractor.cpp
HEADERS += src/AutoRemesher/quadextractor.h

SOURCES += src/AutoRemesher/positionkey.cpp
HEADERS += src/AutoRemesher/positionkey.h

SOURCES += src/AutoRemesher/meshseparator.cpp
HEADERS += src/AutoRemesher/meshseparator.h

SOURCES += src/AutoRemesher/relativeheight.cpp
HEADERS += src/AutoRemesher/relativeheight.h

INCLUDEPATH += thirdparty/openvdb/openvdb-7.0.0
INCLUDEPATH += thirdparty/openexr/openexr-2.4.1
unix {
	LIBS += -Lthirdparty/openvdb/openvdb-7.0.0/build/openvdb -lopenvdb
	LIBS += -Lthirdparty/openexr/openexr-2.4.1/build/IlmBase/Half -lHalf-2_4
    LIBS += -Lthirdparty/zlib/zlib-1.2.11/build -lz
	LIBS += -Lthirdparty/blosc/c-blosc-1.18.1/build/blosc -lblosc
}
win32 {
	LIBS += -Lthirdparty/openvdb/openvdb-7.0.0/build/openvdb/Release -lopenvdb
	LIBS += -Lthirdparty/openexr/openexr-2.4.1/build/IlmBase/Half/Release -lHalf-2_4
}

INCLUDEPATH += thirdparty/geogram/geogram-1.7.5/src/lib
INCLUDEPATH += thirdparty/geogram
win32 {
    DEFINES -= UNICODE
    LIBS += -ladvapi32 -lshell32
}

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/algorithm.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/algorithm.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/command_line.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/command_line.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/environment.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/environment.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/geometry.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/geometry.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/packed_arrays.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/packed_arrays.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/progress.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/progress.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/assert.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/assert.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/command_line_args.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/command_line_args.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/factory.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/factory.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/line_stream.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/line_stream.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/process.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/process.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/quaternion.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/quaternion.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/attributes.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/attributes.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/common.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/common.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/file_system.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/file_system.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/logger.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/logger.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/process_unix.cpp
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/stopwatch.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/stopwatch.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/b_stream.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/b_stream.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/counted.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/counted.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/geofile.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/geofile.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/numeric.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/numeric.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/process_win.cpp
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/string.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/string.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/smart_pointer.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/android_wrapper.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/matrix.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/process_private.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/argused.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/memory.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/psm.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/thread_sync.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/atomics.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/geometry_nd.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/vecg.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/permutation.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/basic/range.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay_3d.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay_3d.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay_tetgen.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay_tetgen.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/LFS.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/LFS.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/periodic.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/periodic.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay_2d.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay_2d.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay_nn.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay_nn.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay_triangle.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/delaunay_triangle.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/parallel_delaunay_3d.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/parallel_delaunay_3d.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/periodic_delaunay_3d.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/periodic_delaunay_3d.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/delaunay/cavity.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/bibliography/bibliography.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/bibliography/bibliography.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/bibliography/embedded_references.cpp

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/parameterization/mesh_global_param.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/parameterization/mesh_global_param.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_AABB.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_AABB.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_frame_field.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_frame_field.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_fill_holes.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_fill_holes.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_geometry.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_geometry.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_halfedges.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_halfedges.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_io.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_io.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_topology.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_topology.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_partition.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_partition.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_preprocessing.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_preprocessing.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_reorder.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_reorder.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_repair.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh_repair.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/mesh/mesh.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/points/co3ne.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/points/co3ne.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/points/colocate.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/points/colocate.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/points/kd_tree.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/points/kd_tree.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/points/nn_search.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/points/nn_search.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/points/principal_axes.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/points/principal_axes.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/expansion_nt.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/expansion_nt.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/lbfgs_optimizers.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/lbfgs_optimizers.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/matrix_util.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/matrix_util.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/multi_precision.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/multi_precision.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/optimizer.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/optimizer.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/predicates.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/numerics/predicates.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_api.cpp
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_os.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_arpack.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_arpack.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_cholmod.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_cholmod.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_cuda.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_cuda.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_iterative_solvers.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_iterative_solvers.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_matrix.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_matrix.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_preconditioners.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_preconditioners.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_superlu.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_superlu.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_64.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_blas.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_blas.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_context.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_context.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_ext.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_linkage.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_mkl.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_mkl.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/NL/nl_private.h

HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_library.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/colormap.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/colormap.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_library.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_library.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_serializer.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_serializer.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_serializer_stb.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_serializer_stb.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/morpho_math.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/morpho_math.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_rasterizer.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_rasterizer.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_serializer_pgm.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_serializer_pgm.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_serializer_xpm.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/image_serializer_xpm.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/image/color.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/convex_cell.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/convex_cell.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/generic_RVD_cell.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/generic_RVD_cell.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/generic_RVD.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/generic_RVD_polygon.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/generic_RVD_vertex.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/generic_RVD_utils.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/integration_simplex.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/integration_simplex.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/RVD_callback.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/RVD_callback.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/CVT.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/CVT.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/generic_RVD_polygon.cpp
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/RVD.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/RVD.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/RVD_mesh_builder.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/voronoi/RVD_mesh_builder.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/third_party/LM7/libmeshb7.c
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/third_party/LM7/libmeshb7.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/third_party/rply/rply.c
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/third_party/rply/rply.h
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/geogram/third_party/rply/rplyfile.h

SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/exploragram/hexdom/basic.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/exploragram/hexdom/basic.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/exploragram/hexdom/frame.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/exploragram/hexdom/frame.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/exploragram/hexdom/spherical_harmonics_l4.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/exploragram/hexdom/spherical_harmonics_l4.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/exploragram/hexdom/polygon.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/exploragram/hexdom/polygon.h
SOURCES += thirdparty/geogram/geogram-1.7.5/src/lib/exploragram/hexdom/quad_cover.cpp
HEADERS += thirdparty/geogram/geogram-1.7.5/src/lib/exploragram/hexdom/quad_cover.h

INCLUDEPATH += thirdparty/tbb/include
unix {
	LIBS += -Lthirdparty/tbb/build2 -ltbbmalloc_proxy_static -ltbbmalloc_static -ltbb_static
    LIBS += -Lthirdparty/zlib/zlib-1.2.11/build -lz
	unix:!macx {
		LIBS += -ldl
	}
}
win32 {
    LIBS += -Lthirdparty/zlib/zlib-1.2.11/build/Release -lzlibstatic
    CONFIG(release, debug|release) LIBS += -Lthirdparty/tbb/build2/Release -ltbb
}

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
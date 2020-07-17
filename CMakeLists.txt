cmake_minimum_required(VERSION 3.10)

#string( REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

project(autoremesher)

add_definitions(
    -D_SCL_SECURE_NO_DEPRECATE
    -D_USE_MATH_DEFINES
    -DINCLUDE_TEMPLATES
)

if (NOT DEFINED CGAL_DIR)
    message(FATAL_ERROR "Couldn't find CGAL_DIR" )
else()
    message("CGAL_DIR: ${CGAL_DIR}")
endif()

if (NOT DEFINED BOOST_INCLUDEDIR)
    message(FATAL_ERROR "Couldn't find BOOST_INCLUDEDIR" )
else()
    message("BOOST_INCLUDEDIR: ${BOOST_INCLUDEDIR}")
endif()

link_directories(
    thirdparty/OpenMesh/OpenMesh-8.1/build/Build/lib
    ${CGAL_DIR}/auxiliary/gmp/lib
)

add_executable(autoremesher 
    src/main.cpp
    src/AutoRemesher/remesher.cpp
    src/AutoRemesher/halfedge.cpp
    src/AutoRemesher/parametrization.cpp
    src/AutoRemesher/skeletonextractor.cpp
    thirdparty/comiso/CoMISo/Solver/EigenLDLTSolver.cc
    thirdparty/comiso/CoMISo/Solver/MISolver.cc
    thirdparty/comiso/CoMISo/Solver/ConstrainedSolver.cc
    thirdparty/comiso/CoMISo/Solver/GMM_Tools.cc
    thirdparty/libQEx/src/MeshExtractor.cc
    thirdparty/libQEx/src/predicates.c
    thirdparty/libQEx/interfaces/c/qex.cc
)

target_include_directories(autoremesher PUBLIC
    include
    thirdparty/comiso
    thirdparty/comiso/Solver
    thirdparty/comiso/CoMISo/ext/gmm-4.2/include
    thirdparty/OpenMesh/OpenMesh-8.1/src
    thirdparty/libQEx/src
    thirdparty/libQEx/interfaces/c
    thirdparty/libigl/include
    thirdparty/eigen
    ${BOOST_INCLUDEDIR}
    ${CGAL_DIR}/include
    ${CGAL_DIR}/auxiliary/gmp/include
)

target_link_libraries(autoremesher PUBLIC 
    OpenMeshCore
    libgmp-10
    libmpfr-4
)
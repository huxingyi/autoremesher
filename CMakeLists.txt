cmake_minimum_required(VERSION 3.10)

project(autoremesher)

add_definitions(
    -D_SCL_SECURE_NO_DEPRECATE
    -D_USE_MATH_DEFINES
    -DINCLUDE_TEMPLATES
)

link_directories(
    thirdparty/OpenMesh/OpenMesh-8.1/build/Build/lib
)

add_executable(autoremesher 
    src/main.cpp
    src/AutoRemesher/remesher.cpp
    src/AutoRemesher/halfedge.cpp
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
)

target_link_libraries(autoremesher PUBLIC 
    OpenMeshCore
)
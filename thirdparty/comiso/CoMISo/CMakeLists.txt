# This is a minimalist cmake file designed to be used
# with libigl. If you need CoMISo for other purposes
# please use the CMakeLists_original.cmake which
# supports all the features of CoMISo

cmake_minimum_required(VERSION 2.8.12)

project(CoMISo)

add_library(CoMISo STATIC
  Solver/EigenLDLTSolver.hh Solver/EigenLDLTSolver.cc
  Solver/MISolver.hh Solver/MISolver.cc
  Solver/ConstrainedSolver.hh Solver/ConstrainedSolver.cc
  Solver/IterativeSolverT.hh Solver/IterativeSolverT.cc
  Solver/GMM_Tools.hh Solver/GMM_Tools.cc)

target_include_directories(CoMISo PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/.."
  "${CMAKE_CURRENT_SOURCE_DIR}/../eigen"
  "${CMAKE_CURRENT_SOURCE_DIR}/ext/gmm-4.2/include")
target_compile_definitions(CoMISo PUBLIC -DINCLUDE_TEMPLATES)

target_link_libraries(CoMISo PUBLIC Eigen3::Eigen)

# Generate position independent code
set_target_properties(CoMISo PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(WIN32)
  target_compile_definitions(CoMISo PRIVATE -D_USE_MATH_DEFINES -DNOMINMAX)
  target_compile_options(CoMISo PUBLIC /D_SCL_SECURE_NO_DEPRECATE)
  add_library(CoMISo_blas STATIC IMPORTED)
  set_target_properties(CoMISo_blas PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/ext/OpenBLAS-v0.2.14-Win64-int64/lib/libopenblas.dll.a.lib")
  target_link_libraries(CoMISo PRIVATE CoMISo_blas)
endif()

if(APPLE)
  find_library(accelerate_library Accelerate)
  target_link_libraries(CoMISo PUBLIC ${accelerate_library})
elseif(UNIX)
  find_package(BLAS REQUIRED)
  target_link_libraries(CoMISo PUBLIC ${BLAS_LIBRARIES})
endif()

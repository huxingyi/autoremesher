# Copyright Contributors to the OpenVDB Project
# SPDX-License-Identifier: MPL-2.0
#
#[=======================================================================[

  CMake Configuration for OpenVDB Unit Tests

#]=======================================================================]

project(OpenVDBUnitTests)
cmake_minimum_required(VERSION 3.3)
# Monitoring <PackageName>_ROOT variables
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

##########################################################################

message(STATUS "----------------------------------------------------")
message(STATUS "----------- Configuring OpenVDBUnitTests -----------")
message(STATUS "----------------------------------------------------")

##########################################################################

# Collect lib dependencies

if(NOT OPENVDB_BUILD_CORE)
  set(OPENVDB_LIB OpenVDB::openvdb)
else()
  set(OPENVDB_LIB openvdb)
endif()

find_package(CppUnit ${MINIMUM_CPPUNIT_VERSION} REQUIRED)

set(OPENVDB_TEST_DEPENDENT_LIBS
  ${OPENVDB_LIB}
  CppUnit::cppunit
)

##########################################################################

if(WIN32 AND OPENVDB_DISABLE_BOOST_IMPLICIT_LINKING)
  add_definitions(-DBOOST_ALL_NO_LIB)
endif()

if(WIN32)
  # Because of implicit linking!
  link_directories(${Boost_LIBRARY_DIR})
endif()

if(WIN32)
  add_definitions(-D_WIN32 -DNOMINMAX -DOPENVDB_DLL)
endif()

##### VDB unit tests

set(UNITTEST_SOURCE_FILES
  main.cc
  TestAttributeArray.cc
  TestAttributeArrayString.cc
  TestAttributeGroup.cc
  TestAttributeSet.cc
  TestBBox.cc
  TestClip.cc
  TestConjGradient.cc
  TestCoord.cc
  TestCpt.cc
  TestCurl.cc
  TestDelayedLoadMetadata.cc
  TestDense.cc
  TestDenseSparseTools.cc
  TestDiagnostics.cc
  TestDivergence.cc
  TestDoubleMetadata.cc
  TestExceptions.cc
  TestFile.cc
  TestFindActiveValues.cc
  TestFloatMetadata.cc
  TestGradient.cc
  TestGrid.cc
  TestGridBbox.cc
  TestGridDescriptor.cc
  TestGridIO.cc
  TestGridTransformer.cc
  TestIndexFilter.cc
  TestIndexIterator.cc
  TestInit.cc
  TestInt32Metadata.cc
  TestInt64Metadata.cc
  TestInternalOrigin.cc
  TestLaplacian.cc
  TestLeaf.cc
  TestLeafBool.cc
  TestLeafIO.cc
  TestLeafManager.cc
  TestLeafMask.cc
  TestLeafOrigin.cc
  TestLevelSetRayIntersector.cc
  TestLevelSetUtil.cc
  TestLinearInterp.cc
  TestMaps.cc
  TestMat4Metadata.cc
  TestMath.cc
  TestMeanCurvature.cc
  TestMeshToVolume.cc
  TestMetadata.cc
  TestMetadataIO.cc
  TestMetaMap.cc
  TestMultiResGrid.cc
  TestName.cc
  TestNodeIterator.cc
  TestNodeManager.cc
  TestNodeMask.cc
  TestParticleAtlas.cc
  TestParticlesToLevelSet.cc
  TestPointAdvect.cc
  TestPointAttribute.cc
  TestPointConversion.cc
  TestPointCount.cc
  TestPointDataLeaf.cc
  TestPointDelete.cc
  TestPointGroup.cc
  TestPointIndexGrid.cc
  TestPointMask.cc
  TestPointMove.cc
  TestPointPartitioner.cc
  TestPointSample.cc
  TestPointScatter.cc
  TestPointsToMask.cc
  TestPoissonSolver.cc
  TestPotentialFlow.cc
  TestPrePostAPI.cc
  TestQuadraticInterp.cc
  TestQuantizedUnitVec.cc
  TestQuat.cc
  TestRay.cc
  TestStats.cc
  TestStream.cc
  TestStreamCompression.cc
  TestStringMetadata.cc
  TestTools.cc
  TestTopologyToLevelSet.cc
  TestTransform.cc
  TestTree.cc
  TestTreeCombine.cc
  TestTreeGetSetValues.cc
  TestTreeIterators.cc
  TestTreeVisitor.cc
  TestTypes.cc
  TestUtil.cc
  TestValueAccessor.cc
  TestVec2Metadata.cc
  TestVec3Metadata.cc
  TestVolumeRayIntersector.cc
  TestVolumeToMesh.cc
  TestVolumeToSpheres.cc
)

add_executable(vdb_test ${UNITTEST_SOURCE_FILES})

if(USE_BLOSC OR OpenVDB_USES_BLOSC)
  # Blosc is a hidden dependency for the core library (not exposed in headers)
  # so we need to manually link it in here to provide header access for the
  # blosc relevant unit tests
  find_package(Blosc ${MINIMUM_BLOSC_VERSION} REQUIRED)
  list(APPEND OPENVDB_TEST_DEPENDENT_LIBS Blosc::blosc)
  target_compile_definitions(vdb_test PRIVATE "-DOPENVDB_USE_BLOSC")
endif()

target_link_libraries(vdb_test
  ${OPENVDB_TEST_DEPENDENT_LIBS}
)

if(OPENVDB_ENABLE_RPATH)
  set(RPATHS "")
  # @todo There is probably a better way to do this for imported targets
  list(APPEND RPATHS
    ${Boost_LIBRARY_DIRS}
    ${IlmBase_LIBRARY_DIRS}
    ${Log4cplus_LIBRARY_DIRS}
    ${Blosc_LIBRARY_DIRS}
    ${Tbb_LIBRARY_DIRS}
    ${CppUnit_LIBRARY_DIRS}
  )
  if(OPENVDB_BUILD_CORE)
    list(APPEND RPATHS ${CMAKE_INSTALL_PREFIX}/lib)
  else()
    list(APPEND RPATHS ${OpenVDB_LIBRARY_DIRS})
  endif()

  list(REMOVE_DUPLICATES RPATHS)

  set_target_properties(vdb_test
    PROPERTIES INSTALL_RPATH "${RPATHS}"
  )
endif()

add_test(vdb_unit_test vdb_test -v)

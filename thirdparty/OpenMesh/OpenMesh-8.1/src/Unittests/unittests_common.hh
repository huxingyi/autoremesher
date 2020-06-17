#ifndef INCLUDE_UNITTESTS_COMMON_HH
#define INCLUDE_UNITTESTS_COMMON_HH

#include <gtest/gtest.h>
#include <OpenMesh/Core/IO/MeshIO.hh>

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

#ifdef TEST_CUSTOM_TRAITS
#include <Unittests/unittests_common_customtraits.hh>
#elif defined(TEST_DOUBLE_TRAITS)
struct CustomTraits : public OpenMesh::DefaultTraitsDouble {
};
#else
struct CustomTraits : public OpenMesh::DefaultTraits {
};
#endif


typedef OpenMesh::TriMesh_ArrayKernelT<CustomTraits> Mesh;

typedef OpenMesh::PolyMesh_ArrayKernelT<CustomTraits> PolyMesh;

/*
 * Simple test setting.
 */

class OpenMeshBase : public testing::Test {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
            
            // Do some initial stuff with the member data here...
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }

    // This member will be accessible in all tests
    Mesh mesh_;  
};

/*
 * Simple test setting.
 */

class OpenMeshBasePoly : public testing::Test {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
            
            // Do some initial stuff with the member data here...
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }

    // This member will be accessible in all tests
   PolyMesh mesh_;  
};



#endif // INCLUDE GUARD

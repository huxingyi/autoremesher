
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <OpenMesh/Tools/Dualizer/meshDualT.hh>

namespace {

class OpenMeshMeshDual : public OpenMeshBasePoly {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
            
            // Do some initial stuff with the member data here...
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }

    // Member already defined in OpenMeshBase
    //Mesh mesh_;  
};

/*
 * ====================================================================
 * Define tests below
 * ====================================================================
 */

/*
 */
TEST_F(OpenMeshMeshDual, Dualize) {

  bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.off");
    
  ASSERT_TRUE(ok);

  PolyMesh* dualMesh;

  dualMesh = OpenMesh::Util::MeshDual(mesh_);

  EXPECT_EQ(15048u, dualMesh->n_vertices()) << "The number of vertices after dual computation is not correct!";
  EXPECT_EQ(22572u, dualMesh->n_edges()) << "The number of edges after dual computation is not correct!";
  EXPECT_EQ(7526u, dualMesh->n_faces()) << "The number of faces after dual computation is not correct!";

  delete(dualMesh);
}
}

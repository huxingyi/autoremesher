
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <OpenMesh/Tools/Utils/StripifierT.hh>

namespace {

class OpenMeshStripify : public OpenMeshBase {

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
TEST_F(OpenMeshStripify, Stripify) {

  bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.off");
    
  ASSERT_TRUE(ok);

  OpenMesh::StripifierT<Mesh> stripifier(mesh_);

  size_t strips = stripifier.stripify();

  EXPECT_EQ(1269u, strips) << "The number of computed strips is not correct!";
  EXPECT_TRUE(stripifier.is_valid()) << "Strips not computed!";

}
}

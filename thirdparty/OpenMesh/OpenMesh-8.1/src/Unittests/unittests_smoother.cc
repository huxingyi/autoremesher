
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <OpenMesh/Tools/Smoother/JacobiLaplaceSmootherT.hh>

namespace {

class OpenMeshSmoother_Poly : public OpenMeshBasePoly {

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

class OpenMeshSmoother_Triangle : public OpenMeshBase {

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
TEST_F(OpenMeshSmoother_Triangle, Smoother_Poly_Laplace) {

  mesh_.clear();


  bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.off");

  ASSERT_TRUE(ok);

  // Check setup
  EXPECT_EQ(7526u, mesh_.n_vertices() ) << "Wrong number of vertices";
  EXPECT_EQ(15048u, mesh_.n_faces() )    << "Wrong number of faces";


  // Initialize subdivider
  OpenMesh::Smoother::JacobiLaplaceSmootherT<Mesh> smoother(mesh_);

  // Just call function to instanciate template
  smoother.set_absolute_local_error(0.5f);

  // Set an error
  smoother.set_relative_local_error(0.1f);

  // Run algorithm with 5 steps
  smoother.smooth(5);


  EXPECT_EQ(7526u, mesh_.n_vertices() ) << "Wrong number of vertices after smoothing?";
  EXPECT_EQ(15048u, mesh_.n_faces() )    << "Wrong number of faces after smoothing?";

}

/*
 * ====================================================================
 * Define tests below
 * ====================================================================
 */

/*
 */
TEST_F(OpenMeshSmoother_Poly, Smoother_Triangle_Laplace) {

  mesh_.clear();


  bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.off");

  ASSERT_TRUE(ok);

  // Check setup
  EXPECT_EQ(7526u, mesh_.n_vertices() ) << "Wrong number of vertices";
  EXPECT_EQ(15048u, mesh_.n_faces() )    << "Wrong number of faces";


  // Initialize subdivider
  OpenMesh::Smoother::JacobiLaplaceSmootherT<PolyMesh> smoother(mesh_);;

  // Just call function to instantiate template
  smoother.set_absolute_local_error(0.5f);

  // Set an error
  smoother.set_relative_local_error(0.1f);

  // Run algorithm with 5 steps
  smoother.smooth(5);


  EXPECT_EQ(7526u, mesh_.n_vertices() ) << "Wrong number of vertices after smoothing?";
  EXPECT_EQ(15048u, mesh_.n_faces() )    << "Wrong number of faces after smoothing?";

}


}

#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshCollapsePoly : public OpenMeshBasePoly {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
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
 * This code tests is_collapse_ok on a double sided triangle. The
 * test mesh comprises three vertices, that are connected to form two 
 * triangles of opposite orientation. All halfedges should be non collapsable.
 */
TEST_F(OpenMeshCollapsePoly, CheckCollapseOkDoublesidedTriangle) {

  mesh_.clear();

  Mesh::VertexHandle vh0 = mesh_.add_vertex(Mesh::Point(0,0,0));
  Mesh::VertexHandle vh1 = mesh_.add_vertex(Mesh::Point(1,0,0));
  Mesh::VertexHandle vh2 = mesh_.add_vertex(Mesh::Point(1,1,0));
  mesh_.add_face(vh0, vh1, vh2);
  mesh_.add_face(vh0, vh2, vh1);



  mesh_.request_vertex_status();
  mesh_.request_face_status();
  mesh_.request_edge_status();

  int collapsable = 0;

  for ( const auto hh : mesh_.all_halfedges() )
  {
    if (mesh_.is_collapse_ok(hh) )
      collapsable++;
  }


  EXPECT_EQ(collapsable,0) << "No collapse should be ok when we have only a double sided Triangle";
}



}

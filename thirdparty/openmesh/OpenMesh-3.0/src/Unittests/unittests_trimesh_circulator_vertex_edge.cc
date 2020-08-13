#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshTrimeshCirculatorVertexEdge : public OpenMeshBase {

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
 * Small VertexEdgeIterator Test without holes in it
 */
TEST_F(OpenMeshTrimeshCirculatorVertexEdge, VertexEdgeIterWithoutHolesIncrement) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[5];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0,-1, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(2,-1, 0));

  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[1]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  /* Test setup:
      0 ==== 2
      |\  0 /|
      | \  / |
      |2  1 3|
      | /  \ |
      |/  1 \|
      3 ==== 4 */

  // Iterate around vertex 1 at the middle (with holes in between)
  Mesh::VertexEdgeIter ve_it  = mesh_.ve_begin(vhandle[1]);
  Mesh::VertexEdgeIter ve_end = mesh_.ve_end(vhandle[1]);
  EXPECT_EQ(5, ve_it->idx() ) << "Index wrong in VertexEdgeIter at initialization";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at initialization";
  ++ve_it ;
  EXPECT_EQ(3, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 1";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at step 1";
  ++ve_it ;
  EXPECT_EQ(0, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 2";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at step 2";
  ++ve_it ;
  EXPECT_EQ(1, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 3";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at step 3";
  ++ve_it ;
  EXPECT_EQ(5, ve_it->idx() ) << "Index wrong in VertexEdgeIter at end";
  EXPECT_FALSE(ve_it.is_valid()) << "Iterator not invalid in VertexEdgeIter at end";
  EXPECT_TRUE( ve_it == ve_end )  << "End iterator for VertexEdgeIter not matching";

  // Iterate around vertex 1 at the middle (with holes in between) with const iterator
  Mesh::ConstVertexEdgeIter cve_it  = mesh_.cve_begin(vhandle[1]);
  Mesh::ConstVertexEdgeIter cve_end = mesh_.cve_end(vhandle[1]);
  EXPECT_EQ(5, cve_it->idx() ) << "Index wrong in ConstVertexEdgeIter at initialization";
  EXPECT_TRUE(cve_it.is_valid()) << "Iterator invalid in ConstVertexEdgeIter at initialization";
  ++cve_it ;
  EXPECT_EQ(3, cve_it->idx() ) << "Index wrong in ConstVertexEdgeIter at step 1";
  EXPECT_TRUE(cve_it.is_valid()) << "Iterator invalid in ConstVertexEdgeIter at step 1";
  ++cve_it ;
  EXPECT_EQ(0, cve_it->idx() ) << "Index wrong in ConstVertexEdgeIter at step 2";
  EXPECT_TRUE(cve_it.is_valid()) << "Iterator invalid in ConstVertexEdgeIter at step 2";
  ++cve_it ;
  EXPECT_EQ(1, cve_it->idx() ) << "Index wrong in ConstVertexEdgeIter at step 3";
  EXPECT_TRUE(cve_it.is_valid()) << "Iterator invalid in ConstVertexEdgeIter at step 3";
  ++cve_it ;
  EXPECT_EQ(5, cve_it->idx() ) << "Index wrong in ConstVertexEdgeIter at end";
  EXPECT_FALSE(cve_it.is_valid()) << "Iterator not invalid in VertexEdgeIter at end";
  EXPECT_TRUE( cve_it == cve_end )  << "End iterator for ConstVertexEdgeIter not matching";
}

/*
 * Small VertexEdgeIterator Test without holes in it
 */
TEST_F(OpenMeshTrimeshCirculatorVertexEdge, VertexEdgeIterBoundaryIncrement) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[5];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0,-1, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(2,-1, 0));

  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[1]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  /* Test setup:
      0 ==== 2
      |\  0 /|
      | \  / |
      |2  1 3|
      | /  \ |
      |/  1 \|
      3 ==== 4 */

  // Iterate around vertex 1 at the middle (with holes in between)
  Mesh::VertexEdgeIter ve_it  = mesh_.ve_begin(vhandle[2]);
  Mesh::VertexEdgeIter ve_end = mesh_.ve_end(vhandle[2]);
  EXPECT_EQ(7, ve_it->idx() ) << "Index wrong in VertexEdgeIter at initialization";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at initialization";
  ++ve_it ;
  EXPECT_EQ(1, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 1";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at step 1";
  ++ve_it ;
  EXPECT_EQ(2, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 2";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at step 2";
  ++ve_it ;
  EXPECT_EQ(7, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 3";
  EXPECT_FALSE(ve_it.is_valid()) << "Iterator not invalid in VertexEdgeIter at end";
  EXPECT_TRUE( ve_it == ve_end )  << "End iterator for VertexEdgeIter not matching";

}



}

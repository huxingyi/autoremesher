#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshTrimeshCirculatorVertexVertex : public OpenMeshBase {

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
 * Small VertexFaceOutgoingHalfedgeIterator Test without holes in it
 */
TEST_F(OpenMeshTrimeshCirculatorVertexVertex, VertexVertexIncrement) {

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
  // Starting vertex is 1->4


  // Iterate around vertex 1 at the middle (with holes in between)
  Mesh::VertexVertexIter vv_it  = mesh_.vv_begin(vhandle[1]);
  Mesh::VertexVertexIter vv_end = mesh_.vv_end(vhandle[1]);

  EXPECT_EQ(4, vv_it->idx() ) << "Index wrong in VertexVertexIter begin at initialization";
  EXPECT_EQ(4, vv_end->idx() ) << "Index wrong in VertexVertexIter end at initialization";
  EXPECT_TRUE(vv_it.is_valid()) << "Iterator invalid in VertexVertexIter at initialization";

  ++vv_it ;

  EXPECT_EQ(3, vv_it->idx() ) << "Index wrong in VertexVertexIter step 1";
  EXPECT_TRUE(vv_it.is_valid()) << "Iterator invalid in VertexVertexIter at step 1";

  ++vv_it ;

  EXPECT_EQ(0, vv_it->idx() ) << "Index wrong in VertexVertexIter step 2";
  EXPECT_TRUE(vv_it.is_valid()) << "Iterator invalid in VertexVertexIter at step 2";

  ++vv_it ;

  EXPECT_EQ(2, vv_it->idx() ) << "Index wrong in VertexVertexIter step 3";
  EXPECT_TRUE(vv_it.is_valid()) << "Iterator invalid in VertexVertexIter at step 3";

  ++vv_it ;

  EXPECT_EQ(4, vv_it->idx() ) << "Index wrong in VertexVertexIter step 4";
  EXPECT_FALSE(vv_it.is_valid()) << "Iterator still valid in VertexVertexIter at step 4";
  EXPECT_TRUE( vv_it == vv_end ) << "Miss matched end iterator";

}

/*
 * Small VertexFaceOutgoingHalfedgeIterator Test at boundary vertex
 */
TEST_F(OpenMeshTrimeshCirculatorVertexVertex, VertexVertexBoundaryIncrement) {

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
  // Starting vertex is 1->4


  // Iterate around vertex 1 at the middle (with holes in between)
  Mesh::VertexVertexIter vv_it  = mesh_.vv_begin(vhandle[2]);
  Mesh::VertexVertexIter vv_end = mesh_.vv_end(vhandle[2]);

  EXPECT_EQ(4, vv_it->idx() ) << "Index wrong in VertexVertexIter begin at initialization";
  EXPECT_EQ(4, vv_end->idx() ) << "Index wrong in VertexVertexIter end at initialization";
  EXPECT_TRUE(vv_it.is_valid()) << "Iterator invalid in VertexVertexIter at initialization";

  ++vv_it ;

  EXPECT_EQ(1, vv_it->idx() ) << "Index wrong in VertexVertexIter step 1";
  EXPECT_TRUE(vv_it.is_valid()) << "Iterator invalid in VertexVertexIter at step 1";

  ++vv_it ;

  EXPECT_EQ(0, vv_it->idx() ) << "Index wrong in VertexVertexIter step 2";
  EXPECT_TRUE(vv_it.is_valid()) << "Iterator invalid in VertexVertexIter at step 2";

  ++vv_it ;

  EXPECT_EQ(4, vv_it->idx() ) << "Index wrong in VertexVertexIter step 3";
  EXPECT_FALSE(vv_it.is_valid()) << "Iterator invalid in VertexVertexIter at step 3";
  EXPECT_TRUE( vv_it == vv_end ) << "Miss matched end iterator";
}
}

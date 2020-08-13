#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshTrimeshCirculatorVertexOHalfEdge : public OpenMeshBase {

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
TEST_F(OpenMeshTrimeshCirculatorVertexOHalfEdge, VertexOutgoingHalfedgeWithoutHolesIncrement) {

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
  // Starting halfedge is 1->4


  // Iterate around vertex 1 at the middle (with holes in between)
  Mesh::VertexOHalfedgeIter voh_it  = mesh_.voh_begin(vhandle[1]);
  Mesh::VertexOHalfedgeIter voh_end = mesh_.voh_end(vhandle[1]);

  EXPECT_EQ(11, voh_it->idx() ) << "Index wrong in VertexOHalfedgeIter begin at initialization";
  EXPECT_EQ(11, voh_end->idx() ) << "Index wrong in VertexOHalfedgeIter end at initialization";
  EXPECT_EQ(3, mesh_.face_handle(*voh_it).idx() ) << "Corresponding face Index wrong in VertexOHalfedgeIter begin at initialization";
  EXPECT_TRUE(voh_it.is_valid()) << "Iterator invalid in VertexOHalfedgeIter at initialization";

  ++voh_it ;

  EXPECT_EQ(6, voh_it->idx() ) << "Index wrong in VertexOHalfedgeIter step 1";
  EXPECT_EQ(1, mesh_.face_handle(*voh_it).idx() ) << "Corresponding face Index wrong in VertexOHalfedgeIter step 1";
  EXPECT_TRUE(voh_it.is_valid()) << "Iterator invalid in VertexOHalfedgeIter at step 1";

  ++voh_it ;

  EXPECT_EQ(1, voh_it->idx() ) << "Index wrong in VertexOHalfedgeIter step 2";
  EXPECT_EQ(2, mesh_.face_handle(*voh_it).idx() ) << "Corresponding face Index wrong in VertexOHalfedgeIter step 2";
  EXPECT_TRUE(voh_it.is_valid()) << "Iterator invalid in VertexOHalfedgeIter at step 2";

  ++voh_it ;

  EXPECT_EQ(2, voh_it->idx() ) << "Index wrong in VertexOHalfedgeIter step 3";
  EXPECT_EQ(0, mesh_.face_handle(*voh_it).idx() ) << "Corresponding face Index wrong in VertexOHalfedgeIter step 3";
  EXPECT_TRUE(voh_it.is_valid()) << "Iterator invalid in VertexOHalfedgeIter at step 3";

  ++voh_it ;

  EXPECT_EQ(11, voh_it->idx() ) << "Index wrong in VertexOHalfedgeIter step 4";
  EXPECT_EQ(3, mesh_.face_handle(*voh_it).idx() ) << "Corresponding face Index wrong in VertexOHalfedgeIter step 4";
  EXPECT_FALSE(voh_it.is_valid()) << "Iterator still valid in VertexOHalfedgeIter at step 4";
  EXPECT_TRUE( voh_it == voh_end ) << "Miss matched end iterator";

  // Iterate around vertex 1 at the middle (with holes in between)
  Mesh::ConstVertexOHalfedgeIter cvoh_it  = mesh_.cvoh_begin(vhandle[1]);
  Mesh::ConstVertexOHalfedgeIter cvoh_end = mesh_.cvoh_end(vhandle[1]);

  EXPECT_EQ(11, cvoh_it->idx() ) << "Index wrong in ConstVertexOHalfedgeIter begin at initialization";
  EXPECT_EQ(11, cvoh_end->idx() ) << "Index wrong in ConstVertexOHalfedgeIter end at initialization";
  EXPECT_EQ(3, mesh_.face_handle(*cvoh_it).idx() ) << "Corresponding face Index wrong in ConstVertexOHalfedgeIter begin at initialization";
  EXPECT_TRUE(cvoh_it.is_valid()) << "Iterator invalid in ConstVertexOHalfedgeIter at initialization";

  ++cvoh_it ;

  EXPECT_EQ(6, cvoh_it->idx() ) << "Index wrong in ConstVertexOHalfedgeIter step 1";
  EXPECT_EQ(1, mesh_.face_handle(*cvoh_it).idx() ) << "Corresponding face Index wrong in ConstVertexOHalfedgeIter step 1";
  EXPECT_TRUE(cvoh_it.is_valid()) << "Iterator invalid in ConstVertexOHalfedgeIter at step 1";

  ++cvoh_it ;

  EXPECT_EQ(1, cvoh_it->idx() ) << "Index wrong in ConstVertexOHalfedgeIter step 2";
  EXPECT_EQ(2, mesh_.face_handle(*cvoh_it).idx() ) << "Corresponding face Index wrong in ConstVertexOHalfedgeIter step 2";
  EXPECT_TRUE(cvoh_it.is_valid()) << "Iterator invalid in ConstVertexOHalfedgeIter at step 2";

  ++cvoh_it ;

  EXPECT_EQ(2, cvoh_it->idx() ) << "Index wrong in ConstVertexOHalfedgeIter step 3";
  EXPECT_EQ(0, mesh_.face_handle(*cvoh_it).idx() ) << "Corresponding face Index wrong in ConstVertexOHalfedgeIter step 3";
  EXPECT_TRUE(cvoh_it.is_valid()) << "Iterator invalid in ConstVertexOHalfedgeIter at step 3";

  ++cvoh_it ;

  EXPECT_EQ(11, cvoh_it->idx() ) << "Index wrong in ConstVertexOHalfedgeIter step 4";
  EXPECT_EQ(3, mesh_.face_handle(*cvoh_it).idx() ) << "Corresponding face Index wrong in ConstVertexOHalfedgeIter step 4";
  EXPECT_FALSE(cvoh_it.is_valid()) << "Iterator still valid in ConstVertexOHalfedgeIter at step 4";
  EXPECT_TRUE( cvoh_it == cvoh_end ) << "Miss matched end iterator";

}

/*
 * Small VertexFaceOutgoingHalfedgeIterator Test
 */
TEST_F(OpenMeshTrimeshCirculatorVertexOHalfEdge, VertexOutgoingHalfedgeBoundaryIncrement) {

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
  // Starting halfedge is 1->4


  // Iterate around vertex 1 at the middle (with holes in between)
  Mesh::VertexOHalfedgeIter voh_it  = mesh_.voh_begin(vhandle[2]);
  Mesh::VertexOHalfedgeIter voh_end = mesh_.voh_end(vhandle[2]);

  EXPECT_EQ(15, voh_it->idx() ) << "Index wrong in VertexOHalfedgeIter begin at initialization";
  EXPECT_EQ(15, voh_end->idx() ) << "Index wrong in VertexOHalfedgeIter end at initialization";
  EXPECT_EQ(-1, mesh_.face_handle(*voh_it).idx() ) << "Corresponding face Index wrong in VertexOHalfedgeIter begin at initialization";
  EXPECT_TRUE(voh_it.is_valid()) << "Iterator invalid in VertexOHalfedgeIter at initialization";

  ++voh_it ;

  EXPECT_EQ(3, voh_it->idx() ) << "Index wrong in VertexOHalfedgeIter step 1";
  EXPECT_EQ(3, mesh_.face_handle(*voh_it).idx() ) << "Corresponding face Index wrong in VertexOHalfedgeIter step 1";
  EXPECT_TRUE(voh_it.is_valid()) << "Iterator invalid in VertexOHalfedgeIter at step 1";

  ++voh_it ;

  EXPECT_EQ(4, voh_it->idx() ) << "Index wrong in VertexOHalfedgeIter step 2";
  EXPECT_EQ(0, mesh_.face_handle(*voh_it).idx() ) << "Corresponding face Index wrong in VertexOHalfedgeIter step 2";
  EXPECT_TRUE(voh_it.is_valid()) << "Iterator invalid in VertexOHalfedgeIter at step 2";

  ++voh_it ;

  EXPECT_EQ(15, voh_it->idx() ) << "Index wrong in VertexOHalfedgeIter step 3";
  EXPECT_EQ(-1, mesh_.face_handle(*voh_it).idx() ) << "Corresponding face Index wrong in VertexOHalfedgeIter step 3";
  EXPECT_FALSE(voh_it.is_valid()) << "Iterator still valid in VertexOHalfedgeIter at step 3";
  EXPECT_TRUE( voh_it == voh_end ) << "Miss matched end iterator";

}



/*
 * Small Test to check dereferencing the iterator
 * No real result
 */
TEST_F(OpenMeshTrimeshCirculatorVertexOHalfEdge, VertexOutgoingHalfedgeDereferenceIncrement) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[5];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0,-1, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(2,-1, 0));

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
  Mesh::VertexOHalfedgeIter voh_it = mesh_.voh_iter(vhandle[1]);

  // TODO: If called without handle, it won't build
  Mesh::EdgeHandle       eh  = mesh_.edge_handle(*voh_it);
  Mesh::HalfedgeHandle   heh = *voh_it;
  Mesh::VertexHandle     vh2 = mesh_.to_vertex_handle(*voh_it);

  EXPECT_EQ(eh.idx()  , 5 )  << "Wrong edge handle after dereferencing";
  EXPECT_EQ(heh.idx() , 11 ) << "Wrong half edge handle after dereferencing";
  EXPECT_EQ(vh2.idx() , 4 )  << "Wrong vertex handle after dereferencing";

}
}

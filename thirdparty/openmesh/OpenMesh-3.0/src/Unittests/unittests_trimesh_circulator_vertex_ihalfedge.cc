#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshTrimeshCirculatorVertexIHalfEdge : public OpenMeshBase {

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
 * Small VertexFaceIterator Test without holes in it
 */
TEST_F(OpenMeshTrimeshCirculatorVertexIHalfEdge, VertexIncomingHalfedgeWithoutHolesIncrement) {

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
  Mesh::VertexIHalfedgeIter vih_it  = mesh_.vih_begin(vhandle[1]);
  Mesh::VertexIHalfedgeIter vih_end = mesh_.vih_end(vhandle[1]);

  EXPECT_EQ(10, vih_it->idx() ) << "Index wrong in VertexIHalfedgeIter begin at initialization";
  EXPECT_EQ(10, vih_end->idx() ) << "Index wrong in VertexIHalfedgeIter end at initialization";
  EXPECT_EQ(1, mesh_.face_handle(*vih_it).idx() ) << "Corresponding face Index wrong in VertexIHalfedgeIter begin at initialization";
  EXPECT_TRUE(vih_it.is_valid()) << "Iterator invalid in VertexIHalfedgeIter at initialization";

  ++vih_it ;

  EXPECT_EQ(7, vih_it->idx() ) << "Index wrong in VertexIHalfedgeIter step 1";
  EXPECT_EQ(2, mesh_.face_handle(*vih_it).idx() ) << "Corresponding face Index wrong in VertexIHalfedgeIter step 1";
  EXPECT_TRUE(vih_it.is_valid()) << "Iterator invalid in VertexIHalfedgeIter at step 1";

  ++vih_it ;

  EXPECT_EQ(0, vih_it->idx() ) << "Index wrong in VertexIHalfedgeIter step 2";
  EXPECT_EQ(0, mesh_.face_handle(*vih_it).idx() ) << "Corresponding face Index wrong in VertexIHalfedgeIter step 2";
  EXPECT_TRUE(vih_it.is_valid()) << "Iterator invalid in VertexIHalfedgeIter at step 2";

  ++vih_it ;

  EXPECT_EQ(3, vih_it->idx() ) << "Index wrong in VertexIHalfedgeIter step 3";
  EXPECT_EQ(3, mesh_.face_handle(*vih_it).idx() ) << "Corresponding face Index wrong in VertexIHalfedgeIter step 3";
  EXPECT_TRUE(vih_it.is_valid()) << "Iterator invalid in VertexIHalfedgeIter at step 3";

  ++vih_it ;

  EXPECT_EQ(10, vih_it->idx() ) << "Index wrong in VertexIHalfedgeIter step 4";
  EXPECT_EQ(1, mesh_.face_handle(*vih_it).idx() ) << "Corresponding face Index wrong in VertexIHalfedgeIter step 4";
  EXPECT_FALSE(vih_it.is_valid()) << "Iterator still valid in VertexIHalfedgeIter at step 4";
  EXPECT_TRUE( vih_it == vih_end ) << "Miss matched end iterator";


  // Iterate around vertex 1 at the middle (with holes in between)
  Mesh::ConstVertexIHalfedgeIter cvih_it  = mesh_.cvih_begin(vhandle[1]);
  Mesh::ConstVertexIHalfedgeIter cvoh_end = mesh_.cvih_end(vhandle[1]);

  EXPECT_EQ(10, cvih_it->idx() ) << "Index wrong in ConstVertexIHalfedgeIter begin at initialization";
  EXPECT_EQ(10, cvoh_end->idx() ) << "Index wrong in ConstVertexIHalfedgeIter end at initialization";
  EXPECT_EQ(1, mesh_.face_handle(*cvih_it).idx() ) << "Corresponding face Index wrong in ConstVertexIHalfedgeIter begin at initialization";
  EXPECT_TRUE(cvih_it.is_valid()) << "Iterator invalid in ConstVertexIHalfedgeIter at initialization";

  ++cvih_it ;

  EXPECT_EQ(7, cvih_it->idx() ) << "Index wrong in ConstVertexIHalfedgeIter step 1";
  EXPECT_EQ(2, mesh_.face_handle(*cvih_it).idx() ) << "Corresponding face Index wrong in ConstVertexIHalfedgeIter step 1";
  EXPECT_TRUE(cvih_it.is_valid()) << "Iterator invalid in ConstVertexIHalfedgeIter at step 1";

  ++cvih_it ;

  EXPECT_EQ(0, cvih_it->idx() ) << "Index wrong in ConstVertexIHalfedgeIter step 2";
  EXPECT_EQ(0, mesh_.face_handle(*cvih_it).idx() ) << "Corresponding face Index wrong in ConstVertexIHalfedgeIter step 2";
  EXPECT_TRUE(cvih_it.is_valid()) << "Iterator invalid in ConstVertexIHalfedgeIter at step 2";

  ++cvih_it ;

  EXPECT_EQ(3, cvih_it->idx() ) << "Index wrong in ConstVertexIHalfedgeIter step 3";
  EXPECT_EQ(3, mesh_.face_handle(*cvih_it).idx() ) << "Corresponding face Index wrong in ConstVertexIHalfedgeIter step 3";
  EXPECT_TRUE(cvih_it.is_valid()) << "Iterator invalid in ConstVertexIHalfedgeIter at step 3";

  ++cvih_it ;

  EXPECT_EQ(10, cvih_it->idx() ) << "Index wrong in ConstVertexIHalfedgeIter step 4";
  EXPECT_EQ(1, mesh_.face_handle(*cvih_it).idx() ) << "Corresponding face Index wrong in ConstVertexIHalfedgeIter step 4";
  EXPECT_FALSE(cvih_it.is_valid()) << "Iterator still valid in ConstVertexIHalfedgeIter at step 4";
  EXPECT_TRUE( cvih_it == cvoh_end ) << "Miss matched end iterator";

}

/*
 * Small VertexFaceOutgoingHalfedgeIterator Test
 */
TEST_F(OpenMeshTrimeshCirculatorVertexIHalfEdge, VertexOIncomingHalfedgeBoundaryIncrement) {

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
  Mesh::VertexIHalfedgeIter vih_it  = mesh_.vih_begin(vhandle[2]);
  Mesh::VertexIHalfedgeIter vih_end = mesh_.vih_end(vhandle[2]);

  EXPECT_EQ(14, vih_it->idx() ) << "Index wrong in VertexIHalfedgeIter begin at initialization";
  EXPECT_EQ(14, vih_end->idx() ) << "Index wrong in VertexIHalfedgeIter end at initialization";
  EXPECT_EQ(3, mesh_.face_handle(*vih_it).idx() ) << "Corresponding face Index wrong in VertexIHalfedgeIter begin at initialization";
  EXPECT_TRUE(vih_it.is_valid()) << "Iterator invalid in VertexIHalfedgeIter at initialization";

  ++vih_it ;

  EXPECT_EQ(2, vih_it->idx() ) << "Index wrong in VertexIHalfedgeIter step 1";
  EXPECT_EQ(0, mesh_.face_handle(*vih_it).idx() ) << "Corresponding face Index wrong in VertexIHalfedgeIter step 1";
  EXPECT_TRUE(vih_it.is_valid()) << "Iterator invalid in VertexIHalfedgeIter at step 1";

  ++vih_it ;

  EXPECT_EQ(5, vih_it->idx() ) << "Index wrong in VertexIHalfedgeIter step 2";
  EXPECT_EQ(-1, mesh_.face_handle(*vih_it).idx() ) << "Corresponding face Index wrong in VertexIHalfedgeIter step 2";
  EXPECT_TRUE(vih_it.is_valid()) << "Iterator invalid in VertexIHalfedgeIter at step 2";

  ++vih_it ;

  EXPECT_EQ(14, vih_it->idx() ) << "Index wrong in VertexIHalfedgeIter step 3";
  EXPECT_EQ(3, mesh_.face_handle(*vih_it).idx() ) << "Corresponding face Index wrong in VertexIHalfedgeIter step 3";
  EXPECT_FALSE(vih_it.is_valid()) << "Iterator still valid in VertexIHalfedgeIter at step 3";
  EXPECT_TRUE( vih_it == vih_end ) << "Miss matched end iterator";


}


/*
 * Small Test to check dereferencing the iterator
 * No real result
 */
TEST_F(OpenMeshTrimeshCirculatorVertexIHalfEdge, VertexIncomingHalfedgeDereferenceIncrement) {

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
  Mesh::VertexIHalfedgeIter vih_it = mesh_.vih_iter(vhandle[1]);

  // TODO: If called without handle, it won't build
  Mesh::EdgeHandle       eh  = mesh_.edge_handle(*vih_it);
  Mesh::HalfedgeHandle   heh = *vih_it;
  Mesh::VertexHandle     vh2 = mesh_.to_vertex_handle(*vih_it);

  EXPECT_EQ(eh.idx()  , 5 )  << "Wrong edge handle after dereferencing";
  EXPECT_EQ(heh.idx() , 10 ) << "Wrong half edge handle after dereferencing";
  EXPECT_EQ(vh2.idx() , 1 )  << "Wrong vertex handle after dereferencing";
}
}

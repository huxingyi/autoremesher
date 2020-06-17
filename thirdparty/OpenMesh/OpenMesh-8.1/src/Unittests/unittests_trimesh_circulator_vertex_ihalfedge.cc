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

/*
 * Test if the end iterator stays invalid after one lap
 * DISABLED as long as the normal iterators using old behavior
 */
//TEST_F(OpenMeshTrimeshCirculatorVertexIHalfEdge, VertexIHalfEdgeIterCheckInvalidationAtEnds) {
//
//  mesh_.clear();
//
//  // Add some vertices
//  Mesh::VertexHandle vhandle[5];
//
//  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
//  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
//  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
//  vhandle[3] = mesh_.add_vertex(Mesh::Point(0,-1, 0));
//  vhandle[4] = mesh_.add_vertex(Mesh::Point(2,-1, 0));
//
//  // Add two faces
//  std::vector<Mesh::VertexHandle> face_vhandles;
//
//  face_vhandles.push_back(vhandle[0]);
//  face_vhandles.push_back(vhandle[1]);
//  face_vhandles.push_back(vhandle[2]);
//  Mesh::FaceHandle fh0 = mesh_.add_face(face_vhandles);
//
//  face_vhandles.clear();
//
//  face_vhandles.push_back(vhandle[1]);
//  face_vhandles.push_back(vhandle[3]);
//  face_vhandles.push_back(vhandle[4]);
//  mesh_.add_face(face_vhandles);
//
//  face_vhandles.clear();
//
//  face_vhandles.push_back(vhandle[0]);
//  face_vhandles.push_back(vhandle[3]);
//  face_vhandles.push_back(vhandle[1]);
//  mesh_.add_face(face_vhandles);
//
//  face_vhandles.clear();
//
//  face_vhandles.push_back(vhandle[2]);
//  face_vhandles.push_back(vhandle[1]);
//  face_vhandles.push_back(vhandle[4]);
//  mesh_.add_face(face_vhandles);
//
//  /* Test setup:
//      0 ==== 2
//      |\  0 /|
//      | \  / |
//      |2  1 3|
//      | /  \ |
//      |/  1 \|
//      3 ==== 4 */
//
//
//  // Check if the end iterator stays invalid after end
//  Mesh::VertexIHalfedgeIter endIter = mesh_.vih_end(vhandle[1]);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  ++endIter ;
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid after increment";
//
//  // Check if the end iterators becomes valid after decrement
//  endIter = mesh_.vih_end(vhandle[1]);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  --endIter;
//  EXPECT_TRUE(endIter.is_valid()) << "EndIter is invalid after decrement";
//  EXPECT_EQ(3,endIter->idx()) << "EndIter points on the wrong element";
//
//
//  // Check if the start iterator decrement is invalid
//  Mesh::VertexIHalfedgeIter startIter = mesh_.vih_begin(vhandle[1]);
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is not valid";
//  --startIter;
//  EXPECT_FALSE(startIter.is_valid()) << "StartIter decrement is not invalid";
//
//  // Check if the start iterator becomes valid
//  ++startIter;
//  EXPECT_TRUE(startIter.is_valid()) << "StarIter is invalid after re-incrementing";
//  EXPECT_EQ(startIter->idx(), mesh_.vih_begin(vhandle[1])->idx()) << "StartIter points on the wrong element";
//
//}

/*
 * Test CW and CCW iterators
 */
TEST_F(OpenMeshTrimeshCirculatorVertexIHalfEdge, CWAndCCWCheck) {

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


  int indices[4] = {14, 5, 2, 14};
  int rev_indices[4];
  std::reverse_copy(indices,indices+4,rev_indices);

  Mesh::VertexHandle vh = vhandle[2];

  //CCW
  Mesh::VertexIHalfedgeCCWIter vih_ccwit  = mesh_.vih_ccwbegin(vh);
  Mesh::VertexIHalfedgeCCWIter vih_ccwend = mesh_.vih_ccwend(vh);
  size_t i = 0;
  for (;vih_ccwit != vih_ccwend; ++vih_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], vih_ccwit->idx()) << "Index wrong in VertexIHalfedgeCCWIter";
  }

  EXPECT_FALSE(vih_ccwit.is_valid()) << "Iterator invalid in VertexIHalfedgeCCWIter at end";
  EXPECT_TRUE( vih_ccwit == vih_ccwend )  << "End iterator for VertexIHalfedgeCCWIter not matching";

  //constant CCW
  Mesh::ConstVertexIHalfedgeCCWIter cvih_ccwit  = mesh_.cvih_ccwbegin(vh);
  Mesh::ConstVertexIHalfedgeCCWIter cvih_ccwend = mesh_.cvih_ccwend(vh);
  i = 0;
  for (;cvih_ccwit != cvih_ccwend; ++cvih_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], cvih_ccwit->idx()) << "Index wrong in ConstVertexIHalfedgeCCWIter";
  }

  EXPECT_FALSE(cvih_ccwit.is_valid()) << "Iterator invalid in ConstVertexIHalfedgeCCWIter at end";
  EXPECT_TRUE( cvih_ccwit == cvih_ccwend )  << "End iterator for ConstVertexIHalfedgeCCWIter not matching";

  //CW
  Mesh::VertexIHalfedgeCWIter vih_cwit  = mesh_.vih_cwbegin(vh);
  Mesh::VertexIHalfedgeCWIter vih_cwend = mesh_.vih_cwend(vh);
  i = 0;
  for (;vih_cwit != vih_cwend; ++vih_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], vih_cwit->idx()) << "Index wrong in VertexIHalfedgeCWIter";
  }
  EXPECT_FALSE(vih_cwit.is_valid()) << "Iterator invalid in VertexIHalfedgeCWIter at end";
  EXPECT_TRUE( vih_cwit == vih_cwend )  << "End iterator for VertexIHalfedgeCWIter not matching";

  //constant CW
  Mesh::ConstVertexIHalfedgeCWIter cvih_cwit  = mesh_.cvih_cwbegin(vh);
  Mesh::ConstVertexIHalfedgeCWIter cvih_cwend = mesh_.cvih_cwend(vh);
  i = 0;
  for (;cvih_cwit != cvih_cwend; ++cvih_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], cvih_cwit->idx()) << "Index wrong in ConstVertexIHalfedgeCWIter";
  }
  EXPECT_FALSE(cvih_cwit.is_valid()) << "Iterator invalid in ConstVertexIHalfedgeCWIter at end";
  EXPECT_TRUE( cvih_cwit == cvih_cwend )  << "End iterator for ConstVertexIHalfedgeCWIter not matching";

  /*
   * conversion properties:
   * a) cw_begin == CWIter(ccw_begin())
   * b) cw_iter->idx() == CCWIter(cw_iter)->idx() for valid iterators
   * c) --cw_iter == CWIter(++ccwIter) for valid iterators
   * d) cw_end == CWIter(ccw_end()) => --cw_end != CWIter(++ccw_end())   *
   */
  Mesh::VertexIHalfedgeCWIter vih_cwIter = mesh_.vih_cwbegin(vh);
  // a)
  EXPECT_TRUE( vih_cwIter == Mesh::VertexIHalfedgeCWIter(mesh_.vih_ccwbegin(vh)) ) << "ccw to cw conversion failed";
  EXPECT_TRUE( Mesh::VertexIHalfedgeCCWIter(vih_cwIter) == mesh_.vih_ccwbegin(vh) ) << "cw to ccw conversion failed";
  // b)
  EXPECT_EQ( vih_cwIter->idx(), Mesh::VertexIHalfedgeCCWIter(vih_cwIter)->idx()) << "iterators doesnt point on the same element";
  // c)
  ++vih_cwIter;
  vih_ccwend = mesh_.vih_ccwend(vh);
  --vih_ccwend;
  EXPECT_EQ(vih_cwIter->idx(),vih_ccwend->idx()) << "iteratoes are not equal after inc/dec";
  // additional conversion check
  vih_ccwend = Mesh::VertexIHalfedgeCCWIter(vih_cwIter);
  EXPECT_EQ(vih_cwIter->idx(),vih_ccwend->idx())<< "iterators doesnt point on the same element";
  // d)
  vih_cwIter = Mesh::VertexIHalfedgeCWIter(mesh_.vih_ccwend(vh));
  EXPECT_FALSE(vih_cwIter.is_valid()) << "end iterator is not invalid";
  EXPECT_TRUE(Mesh::VertexIHalfedgeCCWIter(mesh_.vih_cwend(vh)) ==  mesh_.vih_ccwend(vh)) << "end iterators are not equal";


}


}

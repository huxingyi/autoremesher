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

/*
 * Test if the end iterator stays invalid after one lap
 * DISABLED as long as the normal iterators using old behavior
 */
//TEST_F(OpenMeshTrimeshCirculatorVertexOHalfEdge, VertexOHalfEdgeIterCheckInvalidationAtEnds) {
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
//  Mesh::VertexOHalfedgeIter endIter = mesh_.voh_end(vhandle[1]);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  ++endIter ;
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid after increment";
//
//  endIter = mesh_.voh_end(vhandle[1]);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  --endIter;
//  EXPECT_TRUE(endIter.is_valid()) << "EndIter is invalid after decrement";
//  EXPECT_EQ(2,endIter->idx()) << "EndIter points on the wrong element";
//
//
//  // Check if the start iterator decrement is invalid
//  Mesh::VertexOHalfedgeIter startIter = mesh_.voh_begin(vhandle[1]);
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is not valid";
//  --startIter;
//  EXPECT_FALSE(startIter.is_valid()) << "StartIter decrement is not invalid";
//
//  // Check if the start iterator becomes valid
//  ++startIter;
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is invalid after re-incrementing";
//  EXPECT_EQ(startIter->idx(), mesh_.voh_begin(vhandle[1])->idx()) << "StartIter points on the wrong element";
//
//}

/*
 * Test CW and CCW iterators
 */
TEST_F(OpenMeshTrimeshCirculatorVertexOHalfEdge, CWAndCCWCheck) {

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


  int indices[5] = {11, 2, 1, 6, 11};
  int rev_indices[5];
  std::reverse_copy(indices,indices+5,rev_indices);

  Mesh::VertexHandle vh = vhandle[1];

  //CCW
  Mesh::VertexOHalfedgeCCWIter voh_ccwit  = mesh_.voh_ccwbegin(vh);
  Mesh::VertexOHalfedgeCCWIter voh_ccwend = mesh_.voh_ccwend(vh);
  size_t i = 0;
  for (;voh_ccwit != voh_ccwend; ++voh_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], voh_ccwit->idx()) << "Index wrong in VertexOHalfedgeCCWIter";
  }

  EXPECT_FALSE(voh_ccwit.is_valid()) << "Iterator invalid in VertexOHalfedgeCCWIter at end";
  EXPECT_TRUE( voh_ccwit == voh_ccwend )  << "End iterator for VertexOHalfedgeCCWIter not matching";

  //constant CCW
  Mesh::ConstVertexOHalfedgeCCWIter cvoh_ccwit  = mesh_.cvoh_ccwbegin(vh);
  Mesh::ConstVertexOHalfedgeCCWIter cvoh_ccwend = mesh_.cvoh_ccwend(vh);
  i = 0;
  for (;cvoh_ccwit != cvoh_ccwend; ++cvoh_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], cvoh_ccwit->idx()) << "Index wrong in ConstVertexOHalfedgeCCWIter";
  }

  EXPECT_FALSE(cvoh_ccwit.is_valid()) << "Iterator invalid in ConstVertexOHalfedgeCCWIter at end";
  EXPECT_TRUE( cvoh_ccwit == cvoh_ccwend )  << "End iterator for ConstVertexOHalfedgeCCWIter not matching";

  //CW
  Mesh::VertexOHalfedgeCWIter voh_cwit  = mesh_.voh_cwbegin(vh);
  Mesh::VertexOHalfedgeCWIter voh_cwend = mesh_.voh_cwend(vh);
  i = 0;
  for (;voh_cwit != voh_cwend; ++voh_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], voh_cwit->idx()) << "Index wrong in VertexOHalfedgeCWIter";
  }
  EXPECT_FALSE(voh_cwit.is_valid()) << "Iterator invalid in VertexOHalfedgeCWIter at end";
  EXPECT_TRUE( voh_cwit == voh_cwend )  << "End iterator for VertexOHalfedgeCWIter not matching";

  //constant CW
  Mesh::ConstVertexOHalfedgeCWIter cvoh_cwit  = mesh_.cvoh_cwbegin(vh);
  Mesh::ConstVertexOHalfedgeCWIter cvoh_cwend = mesh_.cvoh_cwend(vh);
  i = 0;
  for (;cvoh_cwit != cvoh_cwend; ++cvoh_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], cvoh_cwit->idx()) << "Index wrong in ConstVertexOHalfedgeCWIter";
  }
  EXPECT_FALSE(cvoh_cwit.is_valid()) << "Iterator invalid in ConstVertexOHalfedgeCWIter at end";
  EXPECT_TRUE( cvoh_cwit == cvoh_cwend )  << "End iterator for ConstVertexOHalfedgeCWIter not matching";

  /*
   * conversion properties:
   * a) cw_begin == CWIter(ccw_begin())
   * b) cw_iter->idx() == CCWIter(cw_iter)->idx() for valid iterators
   * c) --cw_iter == CWIter(++ccwIter) for valid iterators
   * d) cw_end == CWIter(ccw_end()) => --cw_end != CWIter(++ccw_end())   *
   */
  Mesh::VertexOHalfedgeCWIter voh_cwIter = mesh_.voh_cwbegin(vh);
  // a)
  EXPECT_TRUE( voh_cwIter == Mesh::VertexOHalfedgeCWIter(mesh_.voh_ccwbegin(vh)) ) << "ccw to cw conversion failed";
  EXPECT_TRUE( Mesh::VertexOHalfedgeCCWIter(voh_cwIter) == mesh_.voh_ccwbegin(vh) ) << "cw to ccw conversion failed";
  // b)
  EXPECT_EQ( voh_cwIter->idx(), Mesh::VertexOHalfedgeCCWIter(voh_cwIter)->idx()) << "iterators doesnt point on the same element";
  // c)
  ++voh_cwIter;
  voh_ccwend = mesh_.voh_ccwend(vh);
  --voh_ccwend;
  EXPECT_EQ(voh_cwIter->idx(),voh_ccwend->idx()) << "iteratoes are not equal after inc/dec";
  // additional conversion check
  voh_ccwend = Mesh::VertexOHalfedgeCCWIter(voh_cwIter);
  EXPECT_EQ(voh_cwIter->idx(),voh_ccwend->idx())<< "iterators doesnt point on the same element";
  // d)
  voh_cwIter = Mesh::VertexOHalfedgeCWIter(mesh_.voh_ccwend(vh));
  EXPECT_FALSE(voh_cwIter.is_valid()) << "end iterator is not invalid";
  EXPECT_TRUE(Mesh::VertexOHalfedgeCCWIter(mesh_.voh_cwend(vh)) ==  mesh_.voh_ccwend(vh)) << "end iterators are not equal";


}




}

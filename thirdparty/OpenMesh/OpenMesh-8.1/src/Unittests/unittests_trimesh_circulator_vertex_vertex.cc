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


/*
 * Test if the end iterator stays invalid after one lap
 * DISABLED as long as the normal iterators using old behavior
 */
//TEST_F(OpenMeshTrimeshCirculatorVertexVertex, VertexVertexIterCheckInvalidationAtEnds) {
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
//  Mesh::VertexVertexIter endIter = mesh_.vv_end(vhandle[1]);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  ++endIter ;
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid after increment";
//
//  // Check if the end iterators becomes valid after decrement
//  endIter = mesh_.vv_end(vhandle[1]);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  --endIter;
//  EXPECT_TRUE(endIter.is_valid()) << "EndIter is invalid after decrement";
//  EXPECT_EQ(2,endIter->idx()) << "EndIter points on the wrong element";
//
//
//  // Check if the start iterator decrement is invalid
//  Mesh::VertexVertexIter startIter = mesh_.vv_begin(vhandle[1]);
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is not valid";
//  --startIter;
//  EXPECT_FALSE(startIter.is_valid()) << "StartIter decrement is not invalid";
//
//  // Check if the start iterator becomes valid
//  ++startIter;
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is invalid after re-incrementing";
//  EXPECT_EQ(startIter->idx(), mesh_.vv_begin(vhandle[1])->idx()) << "StartIter points on the wrong element";
//
//}

/*
 * Test CW and CCW iterators
 */
TEST_F(OpenMeshTrimeshCirculatorVertexVertex, CWAndCCWCheck) {

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


  int indices[5] = {4, 2, 0, 3, 4};
  int rev_indices[5];
  std::reverse_copy(indices,indices+5,rev_indices);

  Mesh::VertexHandle vh = vhandle[1];

  //CCW
  Mesh::VertexVertexCCWIter vv_ccwit  = mesh_.vv_ccwbegin(vh);
  Mesh::VertexVertexCCWIter vv_ccwend = mesh_.vv_ccwend(vh);
  size_t i = 0;
  for (;vv_ccwit != vv_ccwend; ++vv_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], vv_ccwit->idx()) << "Index wrong in VertexVertexCCWIter";
  }

  EXPECT_FALSE(vv_ccwit.is_valid()) << "Iterator invalid in VertexVertexCCWIter at end";
  EXPECT_TRUE( vv_ccwit == vv_ccwend )  << "End iterator for VertexVertexCCWIter not matching";

  //constant CCW
  Mesh::ConstVertexVertexCCWIter cvv_ccwit  = mesh_.cvv_ccwbegin(vh);
  Mesh::ConstVertexVertexCCWIter cvv_ccwend = mesh_.cvv_ccwend(vh);
  i = 0;
  for (;cvv_ccwit != cvv_ccwend; ++cvv_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], cvv_ccwit->idx()) << "Index wrong in ConstVertexVertexCCWIter";
  }

  EXPECT_FALSE(cvv_ccwit.is_valid()) << "Iterator invalid in ConstVertexVertexCCWIter at end";
  EXPECT_TRUE( cvv_ccwit == cvv_ccwend )  << "End iterator for ConstVertexVertexCCWIter not matching";

  //CW
  Mesh::VertexVertexCWIter vv_cwit  = mesh_.vv_cwbegin(vh);
  Mesh::VertexVertexCWIter vv_cwend = mesh_.vv_cwend(vh);
  i = 0;
  for (;vv_cwit != vv_cwend; ++vv_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], vv_cwit->idx()) << "Index wrong in VertexVertexCWIter";
  }
  EXPECT_FALSE(vv_cwit.is_valid()) << "Iterator invalid in VertexVertexCWIter at end";
  EXPECT_TRUE( vv_cwit == vv_cwend )  << "End iterator for VertexVertexCWIter not matching";

  //constant CW
  Mesh::ConstVertexVertexCWIter cvv_cwit  = mesh_.cvv_cwbegin(vh);
  Mesh::ConstVertexVertexCWIter cvv_cwend = mesh_.cvv_cwend(vh);
  i = 0;
  for (;cvv_cwit != cvv_cwend; ++cvv_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], cvv_cwit->idx()) << "Index wrong in ConstVertexVertexCWIter";
  }
  EXPECT_FALSE(cvv_cwit.is_valid()) << "Iterator invalid in ConstVertexVertexCWIter at end";
  EXPECT_TRUE( cvv_cwit == cvv_cwend )  << "End iterator for ConstVertexVertexCWIter not matching";

  /*
   * conversion properties:
   * a) cw_begin == CWIter(ccw_begin())
   * b) cw_iter->idx() == CCWIter(cw_iter)->idx() for valid iterators
   * c) --cw_iter == CWIter(++ccwIter) for valid iterators
   * d) cw_end == CWIter(ccw_end()) => --cw_end != CWIter(++ccw_end())   *
   */
  Mesh::VertexVertexCWIter vv_cwIter = mesh_.vv_cwbegin(vh);
  // a)
  EXPECT_TRUE( vv_cwIter == Mesh::VertexVertexCWIter(mesh_.vv_ccwbegin(vh)) ) << "ccw to cw convvrsion failed";
  EXPECT_TRUE( Mesh::VertexVertexCCWIter(vv_cwIter) == mesh_.vv_ccwbegin(vh) ) << "cw to ccw convvrsion failed";
  // b)
  EXPECT_EQ( vv_cwIter->idx(), Mesh::VertexVertexCCWIter(vv_cwIter)->idx()) << "iterators doesnt point on the same element";
  // c)
  ++vv_cwIter;
  vv_ccwend = mesh_.vv_ccwend(vh);
  --vv_ccwend;
  EXPECT_EQ(vv_cwIter->idx(),vv_ccwend->idx()) << "iteratoes are not equal after inc/dec";
  // additional conversion check
  vv_ccwend = Mesh::VertexVertexCCWIter(vv_cwIter);
  EXPECT_EQ(vv_cwIter->idx(),vv_ccwend->idx())<< "iterators doesnt point on the same element";
  // d)
  vv_cwIter = Mesh::VertexVertexCWIter(mesh_.vv_ccwend(vh));
  EXPECT_FALSE(vv_cwIter.is_valid()) << "end iterator is not invalid";
  EXPECT_TRUE(Mesh::VertexVertexCCWIter(mesh_.vv_cwend(vh)) ==  mesh_.vv_ccwend(vh)) << "end iterators are not equal";


}



}

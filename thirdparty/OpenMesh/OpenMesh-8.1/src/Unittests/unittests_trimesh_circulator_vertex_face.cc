#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>
#include <algorithm>

namespace {

class OpenMeshTrimeshCirculatorVertexFace : public OpenMeshBase {

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
 * Small VertexFaceIterator Test with holes in it
 *
 * WARNING!!! Basically this is an illegal configuration!
 * But this way we can still detect if it breaks!
 */
TEST_F(OpenMeshTrimeshCirculatorVertexFace, VertexFaceIterWithHolesIncrement) {

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

  /* Test setup:
      0 ==== 2
       \    /
        \  /
          1
        /  \
       /    \
      3 ==== 4 */

  // Iterate around vertex 1 at the middle (with holes in between)
  Mesh::VertexFaceIter vf_it  = mesh_.vf_begin(vhandle[1]);
  Mesh::VertexFaceIter vf_end = mesh_.vf_end(vhandle[1]);
  EXPECT_EQ(0, vf_it->idx() ) << "Index wrong in VertexFaceIter at initialization";
  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at initialization";
  ++vf_it ;
  EXPECT_EQ(1, vf_it->idx() ) << "Index wrong in VertexFaceIter at step 1";
  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at step 1";
  ++vf_it ;
  EXPECT_FALSE(vf_it.is_valid() ) << "Index wrong in VertexFaceIter at end";
  EXPECT_FALSE(vf_it.is_valid()) << "Iterator not invalid in VertexFaceIter at end";
  EXPECT_TRUE( vf_it == vf_end )  << "End iterator for VertexFaceIter not matching";

  // Iterate around vertex 1 at the middle (with holes in between) with const iterator
  Mesh::ConstVertexFaceIter cvf_it  = mesh_.cvf_begin(vhandle[1]);
  Mesh::ConstVertexFaceIter cvf_end = mesh_.cvf_end(vhandle[1]);
  EXPECT_EQ(0, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at initialization";
  EXPECT_TRUE(cvf_it.is_valid()) << "Iterator invalid in ConstVertexFaceIter at initialization";
  ++cvf_it ;
  EXPECT_EQ(1, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at step one";
  EXPECT_TRUE(cvf_it.is_valid()) << "Iterator invalid in ConstVertexFaceIter at step one";
  ++cvf_it ;
  EXPECT_FALSE(cvf_it.is_valid() ) << "Index wrong in ConstVertexFaceIter at end";
  EXPECT_FALSE(cvf_it.is_valid()) << "Iterator not invalid in ConstVertexFaceIter at end";
  EXPECT_TRUE( cvf_it == cvf_end )  << "End iterator for ConstVertexFaceIter not matching";

}



/*
 * Small VertexFaceIterator Test without holes in it
 */
TEST_F(OpenMeshTrimeshCirculatorVertexFace, VertexFaceIterWithoutHolesIncrement) {

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

  mesh_.vf_begin(vhandle[1]);

  // Iterate around vertex 1 at the middle (with holes in between)
  Mesh::VertexFaceIter vf_it  = mesh_.vf_begin(vhandle[1]);
  Mesh::VertexFaceIter vf_end = mesh_.vf_end(vhandle[1]);
  EXPECT_EQ(3, vf_it->idx() ) << "Index wrong in VertexFaceIter at initialization";
  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at initialization";
  ++vf_it ;
  EXPECT_EQ(1, vf_it->idx() ) << "Index wrong in VertexFaceIter at step 1";
  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at step 1";
  ++vf_it ;
  EXPECT_EQ(2, vf_it->idx() ) << "Index wrong in VertexFaceIter at step 2";
  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at step 2";
  ++vf_it ;
  EXPECT_EQ(0, vf_it->idx() ) << "Index wrong in VertexFaceIter at step 3";
  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at step 3";
  ++vf_it ;
  EXPECT_FALSE(vf_it.is_valid()) << "Iterator not invalid in VertexFaceIter at end";
  EXPECT_TRUE( vf_it == vf_end )  << "End iterator for VertexFaceIter not matching";

  // Iterate around vertex 1 at the middle (with holes in between) with const iterator
  Mesh::ConstVertexFaceIter cvf_it  = mesh_.cvf_begin(vhandle[1]);
  Mesh::ConstVertexFaceIter cvf_end = mesh_.cvf_end(vhandle[1]);
  EXPECT_EQ(3, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at initialization";
  EXPECT_TRUE(cvf_it.is_valid()) << "Iterator invalid in ConstVertexFaceIter at initialization";
  ++cvf_it ;
  EXPECT_EQ(1, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at step 1";
  EXPECT_TRUE(cvf_it.is_valid()) << "Iterator invalid in ConstVertexFaceIter at step 1";
  ++cvf_it ;
  EXPECT_EQ(2, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at step 2";
  EXPECT_TRUE(cvf_it.is_valid()) << "Iterator invalid in ConstVertexFaceIter at step 2";
  ++cvf_it ;
  EXPECT_EQ(0, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at step 3";
  EXPECT_TRUE(cvf_it.is_valid()) << "Iterator invalid in ConstVertexFaceIter at step 3";
  ++cvf_it ;
  EXPECT_FALSE(cvf_it.is_valid()) << "Iterator not invalid in VertexFaceIter at end";
  EXPECT_TRUE( cvf_it == cvf_end )  << "End iterator for ConstVertexFaceIter not matching";

}


/*
 * Small VertexFaceIterator Test at a boundary vertex
 */
TEST_F(OpenMeshTrimeshCirculatorVertexFace, VertexFaceIterBoundaryIncrement) {

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
  Mesh::VertexFaceIter vf_it  = mesh_.vf_begin(vhandle[2]);
  Mesh::VertexFaceIter vf_end = mesh_.vf_end(vhandle[2]);
  EXPECT_EQ(3, vf_it->idx() ) << "Index wrong in VertexFaceIter at initialization";
  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at initialization";
  ++vf_it ;
  EXPECT_EQ(0, vf_it->idx() ) << "Index wrong in VertexFaceIter at step 1";
  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at step 1";
  ++vf_it ;
  EXPECT_FALSE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at step 2";
  EXPECT_TRUE( vf_it == vf_end )  << "End iterator for VertexFaceIter not matching";
}

/*
 * Test if the end iterator stays invalid after one lap
 * DISABLED as long as the normal iterators using old behavior
 */
//TEST_F(OpenMeshTrimeshCirculatorVertexFace, VertexFaceIterCheckInvalidationAtEnds) {
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
//  Mesh::VertexFaceIter endIter = mesh_.vf_end(vhandle[1]);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  ++endIter ;
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid after increment";
//
//  // Check if the end iterators becomes valid after decrement
//  endIter = mesh_.vf_end(vhandle[1]);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  --endIter;
//  EXPECT_TRUE(endIter.is_valid()) << "EndIter is invalid after decrement";
//  EXPECT_EQ(0,endIter->idx()) << "EndIter points on the wrong element";
//
//
//  // Check if the start iterator decrement is invalid
//  Mesh::VertexFaceIter startIter = mesh_.vf_begin(vhandle[1]);
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is not valid";
//  --startIter;
//  EXPECT_FALSE(startIter.is_valid()) << "StartIter decrement is not invalid";
//
//  // Check if the start iterator becomes valid
//  ++startIter;
//  EXPECT_TRUE(startIter.is_valid()) << "StarIter is invalid after re-incrementing";
//  EXPECT_EQ(startIter->idx(), mesh_.vf_begin(vhandle[1])->idx()) << "StartIter points on the wrong element";
//
//}

/*
 * VertexFaceIterator Test without holes testing decrement
 * DISABLED as long as the normal iterators using old behavior
 */
//TEST_F(OpenMeshTrimeshCirculatorVertexFace, VertexFaceIterWithoutHolesDecrement) {
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
//  mesh_.add_face(face_vhandles);
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
//  mesh_.vf_begin(vhandle[1]);
//
//  // Iterate around vertex 1 at the middle
//  Mesh::VertexFaceIter vf_it  = mesh_.vf_begin(vhandle[1]);
//  std::advance(vf_it,3);
//  Mesh::VertexFaceIter vf_end = mesh_.vf_begin(vhandle[1]);
//  --vf_end;
//
//  EXPECT_EQ(0, vf_it->idx() ) << "Index wrong in VertexFaceIter at initialization";
//  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at initialization";
//  --vf_it ;
//  EXPECT_EQ(2, vf_it->idx() ) << "Index wrong in VertexFaceIter at step 1";
//  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at step 1";
//  --vf_it ;
//  EXPECT_EQ(1, vf_it->idx() ) << "Index wrong in VertexFaceIter at step 2";
//  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at step 2";
//  --vf_it ;
//  EXPECT_EQ(3, vf_it->idx() ) << "Index wrong in VertexFaceIter at step 3";
//  EXPECT_TRUE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at step 3";
//  --vf_it ;
//  EXPECT_EQ(0, vf_it->idx() ) << "Index wrong in VertexFaceIter at end";
//  EXPECT_FALSE(vf_it.is_valid()) << "Iterator invalid in VertexFaceIter at step 2";
//  EXPECT_TRUE( vf_it == vf_end )  << "End iterator for VertexFaceIter not matching";
//
//  // Iterate around vertex 1 at the middle with const iterator
//  Mesh::ConstVertexFaceIter cvf_it  = mesh_.cvf_begin(vhandle[1]);
//  std::advance(cvf_it,3);
//  Mesh::ConstVertexFaceIter cvf_end = mesh_.cvf_begin(vhandle[1]);
//  --cvf_end;
//
//  EXPECT_EQ(0, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at initialization";
//  EXPECT_TRUE(cvf_it.is_valid()) << "Iterator invalid in ConstVertexFaceIter at initialization";
//  --cvf_it ;
//  EXPECT_EQ(2, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at step 1";
//  EXPECT_TRUE(cvf_it.is_valid()) << "Iterator invalid in ConstVertexFaceIter at step 1";
//  --cvf_it ;
//  EXPECT_EQ(1, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at step 2";
//  EXPECT_TRUE(cvf_it.is_valid()) << "Iterator invalid in ConstVertexFaceIter at step 2";
//  --cvf_it ;
//  EXPECT_EQ(3, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at step 3";
//  EXPECT_TRUE(cvf_it.is_valid()) << "Iterator invalid in ConstVertexFaceIter at step 3";
//  --cvf_it ;
//  EXPECT_EQ(0, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at end";
//  EXPECT_FALSE(cvf_it.is_valid()) << "Iterator invalid in VertexFaceIter at step 2";
//  EXPECT_TRUE( cvf_it == cvf_end )  << "End iterator for ConstVertexFaceIter not matching";
//
//}

/*
 * Test CW and CCW iterators
 */
TEST_F(OpenMeshTrimeshCirculatorVertexFace, CWAndCCWCheck) {

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


  int indices[5] = {3, 0, 2, 1, 3};
  int rev_indices[5];
  std::reverse_copy(indices,indices+5,rev_indices);

  Mesh::VertexHandle vh = vhandle[1];

  //CCW
  Mesh::VertexFaceCCWIter vf_ccwit  = mesh_.vf_ccwbegin(vh);
  Mesh::VertexFaceCCWIter vf_ccwend = mesh_.vf_ccwend(vh);
  size_t i = 0;
  for (;vf_ccwit != vf_ccwend; ++vf_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], vf_ccwit->idx()) << "Index wrong in VertexFaceCCWIter";
  }

  EXPECT_FALSE(vf_ccwit.is_valid()) << "Iterator invalid in VertexFaceCCWIter at end";
  EXPECT_TRUE( vf_ccwit == vf_ccwend )  << "End iterator for VertexFaceCCWIter not matching";

  //constant CCW
  Mesh::ConstVertexFaceCCWIter cvf_ccwit  = mesh_.cvf_ccwbegin(vh);
  Mesh::ConstVertexFaceCCWIter cvf_ccwend = mesh_.cvf_ccwend(vh);
  i = 0;
  for (;cvf_ccwit != cvf_ccwend; ++cvf_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], cvf_ccwit->idx()) << "Index wrong in ConstVertexFaceCCWIter";
  }

  EXPECT_FALSE(cvf_ccwit.is_valid()) << "Iterator invalid in ConstVertexFaceCCWIter at end";
  EXPECT_TRUE( cvf_ccwit == cvf_ccwend )  << "End iterator for ConstVertexFaceCCWIter not matching";

  //CW
  Mesh::VertexFaceCWIter vf_cwit  = mesh_.vf_cwbegin(vh);
  Mesh::VertexFaceCWIter vf_cwend = mesh_.vf_cwend(vh);
  i = 0;
  for (;vf_cwit != vf_cwend; ++vf_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], vf_cwit->idx()) << "Index wrong in VertexFaceCWIter";
  }
  EXPECT_FALSE(vf_cwit.is_valid()) << "Iterator invalid in VertexFaceCWIter at end";
  EXPECT_TRUE( vf_cwit == vf_cwend )  << "End iterator for VertexFaceCWIter not matching";

  //constant CW
  Mesh::ConstVertexFaceCWIter cvf_cwit  = mesh_.cvf_cwbegin(vh);
  Mesh::ConstVertexFaceCWIter cvf_cwend = mesh_.cvf_cwend(vh);
  i = 0;
  for (;cvf_cwit != cvf_cwend; ++cvf_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], cvf_cwit->idx()) << "Index wrong in ConstVertexFaceCWIter";
  }
  EXPECT_FALSE(cvf_cwit.is_valid()) << "Iterator invalid in ConstVertexFaceCWIter at end";
  EXPECT_TRUE( cvf_cwit == cvf_cwend )  << "End iterator for ConstVertexFaceCWIter not matching";

  /*
   * conversion properties:
   * a) cw_begin == CWIter(ccw_begin())
   * b) cw_iter->idx() == CCWIter(cw_iter)->idx() for valid iterators
   * c) --cw_iter == CWIter(++ccwIter) for valid iterators
   * d) cw_end == CWIter(ccw_end()) => --cw_end != CWIter(++ccw_end())   *
   */
  Mesh::VertexFaceCWIter vf_cwIter = mesh_.vf_cwbegin(vh);
  // a)
  EXPECT_TRUE( vf_cwIter == Mesh::VertexFaceCWIter(mesh_.vf_ccwbegin(vh)) ) << "ccw to cw conversion failed";
  EXPECT_TRUE( Mesh::VertexFaceCCWIter(vf_cwIter) == mesh_.vf_ccwbegin(vh) ) << "cw to ccw conversion failed";
  // b)
  EXPECT_EQ( vf_cwIter->idx(), Mesh::VertexFaceCCWIter(vf_cwIter)->idx()) << "iterators doesnt point on the same element";
  // c)
  ++vf_cwIter;
  vf_ccwend = mesh_.vf_ccwend(vh);
  --vf_ccwend;
  EXPECT_EQ(vf_cwIter->idx(),vf_ccwend->idx()) << "iteratoes are not equal after inc/dec";
  // additional conversion check
  vf_ccwend = Mesh::VertexFaceCCWIter(vf_cwIter);
  EXPECT_EQ(vf_cwIter->idx(),vf_ccwend->idx())<< "iterators doesnt point on the same element";
  // d)
  vf_cwIter = Mesh::VertexFaceCWIter(mesh_.vf_ccwend(vh));
  EXPECT_FALSE(vf_cwIter.is_valid()) << "end iterator is not invalid";
  EXPECT_TRUE(Mesh::VertexFaceCCWIter(mesh_.vf_cwend(vh)) ==  mesh_.vf_ccwend(vh)) << "end iterators are not equal";


}


}

#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>
#include <algorithm>

namespace {

class OpenMeshTrimeshCirculatorFaceHalfEdge : public OpenMeshBase {

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
 * Small FaceHalfedgeIterator Test
 */
//TEST_F(OpenMeshTrimeshCirculatorFaceHalfEdge, FaceHalfedgeIterWithoutHolesIncrement) {
//
//  mesh_.clear();
//
//  // Add some vertices
//  Mesh::VertexHandle vhandle[6];
//
//  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
//  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
//  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
//  vhandle[3] = mesh_.add_vertex(Mesh::Point(3, 0, 0));
//  vhandle[4] = mesh_.add_vertex(Mesh::Point(4, 1, 0));
//  vhandle[5] = mesh_.add_vertex(Mesh::Point(2,-1, 0));
//
//  // Add three faces
//  std::vector<Mesh::VertexHandle> face_vhandles;
//
//  face_vhandles.push_back(vhandle[0]);
//  face_vhandles.push_back(vhandle[1]);
//  face_vhandles.push_back(vhandle[2]);
//  mesh_.add_face(face_vhandles);
//
//  face_vhandles.clear();
//
//  face_vhandles.push_back(vhandle[2]);
//  face_vhandles.push_back(vhandle[1]);
//  face_vhandles.push_back(vhandle[3]);
//  mesh_.add_face(face_vhandles);
//
//  face_vhandles.clear();
//
//  face_vhandles.push_back(vhandle[2]);
//  face_vhandles.push_back(vhandle[3]);
//  face_vhandles.push_back(vhandle[4]);
//  mesh_.add_face(face_vhandles);
//
//  face_vhandles.clear();
//
//  face_vhandles.push_back(vhandle[1]);
//  face_vhandles.push_back(vhandle[5]);
//  face_vhandles.push_back(vhandle[3]);
//  mesh_.add_face(face_vhandles);
//
//  /* Test setup:
//   *
//   * 0 ------ 2 ------ 4
//   *  \      / \      /
//   *   \  0 /   \  2 /
//   *    \  /  1  \  /
//   *     1 ------- 3
//   *      \       /
//   *       \  3  /
//   *        \   /
//   *         \ /
//   *          5
//   */
//
//
//  Mesh::FaceHalfedgeIter fh_it  = mesh_.fh_begin(mesh_.face_handle(1));
//  Mesh::FaceHalfedgeIter fh_end = mesh_.fh_end(mesh_.face_handle(1));
//
//  EXPECT_EQ(8, fh_it->idx() ) << "Index wrong in FaceHalfedgeIter at initialization";
//  EXPECT_TRUE(fh_it.is_valid()) << "Iterator invalid in FaceHalfedgeIter at initialization";
//  ++fh_it;
//  EXPECT_EQ(3, fh_it->idx() ) << "Index wrong in FaceHalfedgeIter at step 1";
//  EXPECT_TRUE(fh_it.is_valid()) << "Iterator invalid in FaceHalfedgeIter at step 1";
//  ++fh_it;
//  EXPECT_EQ(6, fh_it->idx() ) << "Index wrong in FaceHalfedgeIter at step 2";
//  EXPECT_TRUE(fh_it.is_valid()) << "Iterator invalid in FaceHalfedgeIter at step 2";
//  ++fh_it;
//  EXPECT_EQ(8, fh_it->idx() ) << "Index wrong in FaceHalfedgeIter at end";
//  EXPECT_FALSE(fh_it.is_valid()) << "Iterator invalid in FaceHalfedgeIter at end";
//  EXPECT_TRUE( fh_it == fh_end )  << "End iterator for FaceHalfedgeIter not matching";
//
//  Mesh::ConstFaceHalfedgeIter cfh_it  = mesh_.cfh_begin(mesh_.face_handle(1));
//  Mesh::ConstFaceHalfedgeIter cfh_end = mesh_.cfh_end(mesh_.face_handle(1));
//
//  EXPECT_EQ(8, cfh_it->idx() ) << "Index wrong in ConstFaceHalfedgeIter at initialization";
//  EXPECT_TRUE(cfh_it.is_valid()) << "Iterator invalid in ConstFaceHalfedgeIter at initialization";
//  ++cfh_it;
//  EXPECT_EQ(3, cfh_it->idx() ) << "Index wrong in ConstFaceHalfedgeIter at step 1";
//  EXPECT_TRUE(cfh_it.is_valid()) << "Iterator invalid in ConstFaceHalfedgeIter at step 1";
//  ++cfh_it;
//  EXPECT_EQ(6, cfh_it->idx() ) << "Index wrong in ConstFaceHalfedgeIter at step 2";
//  EXPECT_TRUE(cfh_it.is_valid()) << "Iterator invalid in ConstFaceHalfedgeIter at step 2";
//  ++cfh_it;
//  EXPECT_EQ(8, cfh_it->idx() ) << "Index wrong in ConstFaceHalfedgeIter at end";
//  EXPECT_FALSE(cfh_it.is_valid()) << "Iterator invalid in ConstFaceHalfedgeIter at end";
//  EXPECT_TRUE( cfh_it == cfh_end )  << "End iterator for ConstFaceHalfedgeIter not matching";
//
//}

/*
 * test CW and CCW iterators
 */
TEST_F(OpenMeshTrimeshCirculatorFaceHalfEdge, CWAndCCWTest) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[6];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(3, 0, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(4, 1, 0));
  vhandle[5] = mesh_.add_vertex(Mesh::Point(2,-1, 0));

  // Add three faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  /* Test setup:
   *
   * 0 ------ 2 ------ 4
   *  \      / \      /
   *   \  0 /   \  2 /
   *    \  /  1  \  /
   *     1 ------- 3
   *      \       /
   *       \  3  /
   *        \   /
   *         \ /
   *          5
   */


  int indices[4] = {8, 3, 6, 8};
  int rev_indices[4];
  std::reverse_copy(indices,indices+4,rev_indices);

  //CCW
  Mesh::FaceHalfedgeCCWIter fh_ccwit  = mesh_.fh_ccwbegin(mesh_.face_handle(1));
  Mesh::FaceHalfedgeCCWIter fh_ccwend = mesh_.fh_ccwend(mesh_.face_handle(1));
  size_t i = 0;
  for (;fh_ccwit != fh_ccwend; ++fh_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], fh_ccwit->idx()) << "Index wrong in FaceHalfedgeIter";
  }

  EXPECT_FALSE(fh_ccwit.is_valid()) << "Iterator invalid in FaceHalfedgeIter at end";
  EXPECT_TRUE( fh_ccwit == fh_ccwend )  << "End iterator for FaceHalfedgeIter not matching";

  //constant CCW
  Mesh::ConstFaceHalfedgeCCWIter cfh_ccwit  = mesh_.cfh_ccwbegin(mesh_.face_handle(1));
  Mesh::ConstFaceHalfedgeCCWIter cfh_ccwend = mesh_.cfh_ccwend(mesh_.face_handle(1));
  i = 0;
  for (;cfh_ccwit != cfh_ccwend; ++cfh_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], cfh_ccwit->idx()) << "Index wrong in ConstFaceHalfedgeIter";
  }

  EXPECT_FALSE(cfh_ccwit.is_valid()) << "Iterator invalid in ConstFaceHalfedgeIter at end";
  EXPECT_TRUE( cfh_ccwit == cfh_ccwend )  << "End iterator for ConstFaceHalfedgeIter not matching";

  //CW
  Mesh::FaceHalfedgeCWIter fh_cwit  = mesh_.fh_cwbegin(mesh_.face_handle(1));
  Mesh::FaceHalfedgeCWIter fh_cwend = mesh_.fh_cwend(mesh_.face_handle(1));
  i = 0;
  for (;fh_cwit != fh_cwend; ++fh_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], fh_cwit->idx()) << "Index wrong in FaceHalfedgeCWIter";
  }
  EXPECT_FALSE(fh_cwit.is_valid()) << "Iterator invalid in FaceHalfedgeCWIter at end";
  EXPECT_TRUE( fh_cwit == fh_cwend )  << "End iterator for FaceHalfedgeCWIter not matching";

  //constant CW
  Mesh::ConstFaceHalfedgeCWIter cfh_cwit  = mesh_.cfh_cwbegin(mesh_.face_handle(1));
  Mesh::ConstFaceHalfedgeCWIter cfh_cwend = mesh_.cfh_cwend(mesh_.face_handle(1));
  i = 0;
  for (;cfh_cwit != cfh_cwend; ++cfh_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], cfh_cwit->idx()) << "Index wrong in ConstFaceHalfedgeCWIter";
  }
  EXPECT_FALSE(cfh_cwit.is_valid()) << "Iterator invalid in ConstFaceHalfedgeCWIter at end";
  EXPECT_TRUE( cfh_cwit == cfh_cwend )  << "End iterator for ConstFaceHalfedgeCWIter not matching";

  /*
   * conversion properties:
   * a) cw_begin == CWIter(ccw_begin())
   * b) cw_iter->idx() == CCWIter(cw_iter)->idx() for valid iterators
   * c) --cw_iter == CWIter(++ccwIter) for valid iterators
   * d) cw_end == CWIter(ccw_end()) => --cw_end != CWIter(++ccw_end())   *
   */
  Mesh::FaceHalfedgeCWIter fh_cwIter = mesh_.fh_cwbegin(mesh_.face_handle(1));
  // a)
  EXPECT_TRUE( fh_cwIter == Mesh::FaceHalfedgeCWIter(mesh_.fh_ccwbegin(mesh_.face_handle(1))) ) << "ccw to cw conversion failed";
  EXPECT_TRUE( Mesh::FaceHalfedgeCCWIter(fh_cwIter) == mesh_.fh_ccwbegin(mesh_.face_handle(1)) ) << "cw to ccw conversion failed";
  // b)
  EXPECT_EQ( fh_cwIter->idx(), Mesh::FaceHalfedgeCCWIter(fh_cwIter)->idx()) << "iterators doesnt point on the same element";
  // c)
  ++fh_cwIter;
  fh_ccwend = mesh_.fh_ccwend(mesh_.face_handle(1));
  --fh_ccwend;
  EXPECT_EQ(fh_cwIter->idx(),fh_ccwend->idx()) << "iteratoes are not equal after inc/dec";
  // additional conversion check
  fh_ccwend = Mesh::FaceHalfedgeCCWIter(fh_cwIter);
  EXPECT_EQ(fh_cwIter->idx(),fh_ccwend->idx())<< "iterators doesnt point on the same element";
  // d)
  fh_cwIter = Mesh::FaceHalfedgeCWIter(mesh_.fh_ccwend(mesh_.face_handle(1)));
  EXPECT_FALSE(fh_cwIter.is_valid()) << "end iterator is not invalid";
  EXPECT_TRUE(Mesh::FaceHalfedgeCCWIter(mesh_.fh_cwend(mesh_.face_handle(1))) ==  mesh_.fh_ccwend(mesh_.face_handle(1))) << "end iterators are not equal";


}


/*
 * Test if the end iterator stays invalid after one lap
 */
//TEST_F(OpenMeshTrimeshCirculatorFaceHalfEdge, FaceHalfedgeIterCheckInvalidationAtEnds) {
//
//  mesh_.clear();
//
//   // Add some vertices
//   Mesh::VertexHandle vhandle[5];
//
//   vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
//   vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
//   vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
//   vhandle[3] = mesh_.add_vertex(Mesh::Point(0,-1, 0));
//   vhandle[4] = mesh_.add_vertex(Mesh::Point(2,-1, 0));
//
//   // Add two faces
//   std::vector<Mesh::VertexHandle> face_vhandles;
//
//   face_vhandles.push_back(vhandle[0]);
//   face_vhandles.push_back(vhandle[1]);
//   face_vhandles.push_back(vhandle[2]);
//   Mesh::FaceHandle fh0 = mesh_.add_face(face_vhandles);
//
//   face_vhandles.clear();
//
//   face_vhandles.push_back(vhandle[1]);
//   face_vhandles.push_back(vhandle[3]);
//   face_vhandles.push_back(vhandle[4]);
//   mesh_.add_face(face_vhandles);
//
//   face_vhandles.clear();
//
//   face_vhandles.push_back(vhandle[0]);
//   face_vhandles.push_back(vhandle[3]);
//   face_vhandles.push_back(vhandle[1]);
//   mesh_.add_face(face_vhandles);
//
//   face_vhandles.clear();
//
//   face_vhandles.push_back(vhandle[2]);
//   face_vhandles.push_back(vhandle[1]);
//   face_vhandles.push_back(vhandle[4]);
//   mesh_.add_face(face_vhandles);
//
//   /* Test setup:
//       0 ==== 2
//       |\  0 /|
//       | \  / |
//       |2  1 3|
//       | /  \ |
//       |/  1 \|
//       3 ==== 4 */
//
//
//  // Check if the end iterator stays invalid after end
//  Mesh::FaceHalfedgeIter endIter = mesh_.fh_end(fh0);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  ++endIter ;
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid after increment";
//
//  // Check if the end iterators becomes valid after decrement
//  endIter = mesh_.fh_end(fh0);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  --endIter;
//  EXPECT_TRUE(endIter.is_valid()) << "EndIter is invalid after decrement";
//  EXPECT_EQ(2,endIter->idx()) << "EndIter points on the wrong element";
//
//
//  // Check if the start iterator decrement is invalid
//  Mesh::FaceHalfedgeIter startIter = mesh_.fh_begin(fh0);
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is not valid";
//  --startIter;
//  EXPECT_FALSE(startIter.is_valid()) << "StartIter decrement is not invalid";
//
//  // Check if the start iterator becomes valid
//  ++startIter;
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is invalid after re-incrementing";
//  EXPECT_EQ(startIter->idx(), mesh_.fh_begin(fh0)->idx()) << "StartIter points on the wrong element";
//
//}

}

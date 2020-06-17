#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>
#include <algorithm>

namespace {

class OpenMeshTrimeshCirculatorFaceEdge : public OpenMeshBase {

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
 * Small FaceEdgeIterator Test
 */
TEST_F(OpenMeshTrimeshCirculatorFaceEdge, FaceEdgeIterWithoutHolesIncrement) {

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


  Mesh::FaceEdgeIter fe_it  = mesh_.fe_begin(mesh_.face_handle(1));
  Mesh::FaceEdgeIter fe_end = mesh_.fe_end(mesh_.face_handle(1));

  EXPECT_EQ(4, fe_it->idx() ) << "Index wrong in FaceEdgeIter at initialization";
  EXPECT_TRUE(fe_it.is_valid()) << "Iterator invalid in FaceEdgeIter at initialization";
  ++fe_it;
  EXPECT_EQ(1, fe_it->idx() ) << "Index wrong in FaceEdgeIter at step 1";
  EXPECT_TRUE(fe_it.is_valid()) << "Iterator invalid in FaceEdgeIter at step 1";
  ++fe_it;
  EXPECT_EQ(3, fe_it->idx() ) << "Index wrong in FaceEdgeIter at step 2";
  EXPECT_TRUE(fe_it.is_valid()) << "Iterator invalid in FaceEdgeIter at step 2";
  ++fe_it;
  EXPECT_EQ(4, fe_it->idx() ) << "Index wrong in FaceEdgeIter at end";
  EXPECT_FALSE(fe_it.is_valid()) << "Iterator invalid in FaceEdgeIter at end";
  EXPECT_TRUE( fe_it == fe_end )  << "End iterator for FaceEdgeIter not matching";

  Mesh::ConstFaceEdgeIter cfe_it  = mesh_.cfe_begin(mesh_.face_handle(1));
  Mesh::ConstFaceEdgeIter cfe_end = mesh_.cfe_end(mesh_.face_handle(1));

  EXPECT_EQ(4, cfe_it->idx() ) << "Index wrong in ConstFaceEdgeIter at initialization";
  EXPECT_TRUE(cfe_it.is_valid()) << "Iterator invalid in ConstFaceEdgeIter at initialization";
  ++cfe_it;
  EXPECT_EQ(1, cfe_it->idx() ) << "Index wrong in ConstFaceEdgeIter at step 1";
  EXPECT_TRUE(cfe_it.is_valid()) << "Iterator invalid in ConstFaceEdgeIter at step 1";
  ++cfe_it;
  EXPECT_EQ(3, cfe_it->idx() ) << "Index wrong in ConstFaceEdgeIter at step 2";
  EXPECT_TRUE(cfe_it.is_valid()) << "Iterator invalid in ConstFaceEdgeIter at step 2";
  ++cfe_it;
  EXPECT_EQ(4, cfe_it->idx() ) << "Index wrong in ConstFaceEdgeIter at end";
  EXPECT_FALSE(cfe_it.is_valid()) << "Iterator invalid in ConstFaceEdgeIter at end";
  EXPECT_TRUE( cfe_it == cfe_end )  << "End iterator for ConstFaceEdgeIter not matching";

}
}

/*
 * test CW and CCW iterators
 */
TEST_F(OpenMeshTrimeshCirculatorFaceEdge, CWAndCCWTest) {

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


  int indices[4] = {4, 1, 3, 4};
  int rev_indices[4];
  std::reverse_copy(indices,indices+4,rev_indices);

  //CCW
  Mesh::FaceEdgeCCWIter fe_ccwit  = mesh_.fe_ccwbegin(mesh_.face_handle(1));
  Mesh::FaceEdgeCCWIter fe_ccwend = mesh_.fe_ccwend(mesh_.face_handle(1));
  size_t i = 0;
  for (;fe_ccwit != fe_ccwend; ++fe_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], fe_ccwit->idx()) << "Index wrong in FaceEdgeCCWIter";
  }

  EXPECT_FALSE(fe_ccwit.is_valid()) << "Iterator invalid in FaceEdgeCCWIter at end";
  EXPECT_TRUE( fe_ccwit == fe_ccwend )  << "End iterator for FaceEdgeCCWIter not matching";

  //constant CCW
  Mesh::ConstFaceEdgeCCWIter cfe_ccwit  = mesh_.cfe_ccwbegin(mesh_.face_handle(1));
  Mesh::ConstFaceEdgeCCWIter cfe_ccwend = mesh_.cfe_ccwend(mesh_.face_handle(1));
  i = 0;
  for (;cfe_ccwit != cfe_ccwend; ++cfe_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], cfe_ccwit->idx()) << "Index wrong in ConstFaceEdgeCCWIter";
  }

  EXPECT_FALSE(cfe_ccwit.is_valid()) << "Iterator invalid in ConstFaceEdgeCCWIter at end";
  EXPECT_TRUE( cfe_ccwit == cfe_ccwend )  << "End iterator for ConstFaceEdgeCCWIter not matching";

  //CW
  Mesh::FaceEdgeCWIter fe_cwit  = mesh_.fe_cwbegin(mesh_.face_handle(1));
  Mesh::FaceEdgeCWIter fe_cwend = mesh_.fe_cwend(mesh_.face_handle(1));
  i = 0;
  for (;fe_cwit != fe_cwend; ++fe_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], fe_cwit->idx()) << "Index wrong in FaceEdgeCWIter";
  }
  EXPECT_FALSE(fe_cwit.is_valid()) << "Iterator invalid in FaceEdgeCWIter at end";
  EXPECT_TRUE( fe_cwit == fe_cwend )  << "End iterator for FaceEdgeCWIter not matching";

  //constant CW
  Mesh::ConstFaceEdgeCWIter cfe_cwit  = mesh_.cfe_cwbegin(mesh_.face_handle(1));
  Mesh::ConstFaceEdgeCWIter cfe_cwend = mesh_.cfe_cwend(mesh_.face_handle(1));
  i = 0;
  for (;cfe_cwit != cfe_cwend; ++cfe_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], cfe_cwit->idx()) << "Index wrong in ConstFaceEdgeCWIter";
  }
  EXPECT_FALSE(cfe_cwit.is_valid()) << "Iterator invalid in ConstFaceEdgeCWIter at end";
  EXPECT_TRUE( cfe_cwit == cfe_cwend )  << "End iterator for ConstFaceEdgeCWIter not matching";

  /*
   * conversion properties:
   * a) cw_begin == CWIter(ccw_begin())
   * b) cw_iter->idx() == CCWIter(cw_iter)->idx() for valid iterators
   * c) --cw_iter == CWIter(++ccwIter) for valid iterators
   * d) cw_end == CWIter(ccw_end()) => --cw_end != CWIter(++ccw_end())   *
   */
  Mesh::FaceEdgeCWIter fe_cwIter = mesh_.fe_cwbegin(mesh_.face_handle(1));
  // a)
  EXPECT_TRUE( fe_cwIter == Mesh::FaceEdgeCWIter(mesh_.fe_ccwbegin(mesh_.face_handle(1))) ) << "ccw to cw conversion failed";
  EXPECT_TRUE( Mesh::FaceEdgeCCWIter(fe_cwIter) == mesh_.fe_ccwbegin(mesh_.face_handle(1)) ) << "cw to ccw conversion failed";
  // b)
  EXPECT_EQ( fe_cwIter->idx(), Mesh::FaceEdgeCCWIter(fe_cwIter)->idx()) << "iterators doesnt point on the same element";
  // c)
  ++fe_cwIter;
  fe_ccwend = mesh_.fe_ccwend(mesh_.face_handle(1));
  --fe_ccwend;
  EXPECT_EQ(fe_cwIter->idx(),fe_ccwend->idx()) << "iteratoes are not equal after inc/dec";
  // additional conversion check
  fe_ccwend = Mesh::FaceEdgeCCWIter(fe_cwIter);
  EXPECT_EQ(fe_cwIter->idx(),fe_ccwend->idx())<< "iterators doesnt point on the same element";
  // d)
  fe_cwIter = Mesh::FaceEdgeCWIter(mesh_.fe_ccwend(mesh_.face_handle(1)));
  EXPECT_FALSE(fe_cwIter.is_valid()) << "end iterator is not invalid";
  EXPECT_TRUE(Mesh::FaceEdgeCCWIter(mesh_.fe_cwend(mesh_.face_handle(1))) ==  mesh_.fe_ccwend(mesh_.face_handle(1))) << "end iterators are not equal";

}


/*
 * Test if the end iterator stays invalid after one lap
 */
//TEST_F(OpenMeshTrimeshCirculatorFaceEdge, FaceEdgeIterCheckInvalidationAtEnds) {
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
//  Mesh::FaceEdgeIter endIter = mesh_.fe_end(fh0);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  ++endIter ;
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid after increment";
//
//  // Check if the end iterators becomes valid after decrement
//  endIter = mesh_.fe_end(fh0);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  --endIter;
//  EXPECT_TRUE(endIter.is_valid()) << "EndIter is invalid after decrement";
//  EXPECT_EQ(1,endIter->idx()) << "EndIter points on the wrong element";
//
//
//  // Check if the start iterator decrement is invalid
//  Mesh::FaceEdgeIter startIter = mesh_.fe_begin(fh0);
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is not valid";
//  --startIter;
//  EXPECT_FALSE(startIter.is_valid()) << "StartIter decrement is not invalid";
//
//  // Check if the start iterator becomes valid
//  ++startIter;
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is invalid after re-incrementing";
//  EXPECT_EQ(startIter->idx(), mesh_.fe_begin(fh0)->idx()) << "StartIter points on the wrong element";
//
//}


/*
 * Small FaceEdgeIterator Test
 */
/*
TEST_F(OpenMeshTrimeshCirculatorFaceEdge, FaceEdgeIterWithoutHolesDecrement) {

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
*/
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

/*
  Mesh::FaceEdgeIter fe_it  = mesh_.fe_begin(mesh_.face_handle(1));
  Mesh::FaceEdgeIter fe_end = mesh_.fe_end(mesh_.face_handle(1));

  EXPECT_EQ(4, fe_it->idx() ) << "Index wrong in FaceEdgeIter at initialization";
  EXPECT_TRUE(fe_it) << "Iterator invalid in FaceEdgeIter at initialization";
  --fe_it;
  EXPECT_EQ(1, fe_it->idx() ) << "Index wrong in FaceEdgeIter at step 1";
  EXPECT_TRUE(fe_it) << "Iterator invalid in FaceEdgeIter at step 1";
  --fe_it;
  EXPECT_EQ(3, fe_it->idx() ) << "Index wrong in FaceEdgeIter at step 2";
  EXPECT_TRUE(fe_it) << "Iterator invalid in FaceEdgeIter at step 2";
  --fe_it;
  EXPECT_EQ(4, fe_it->idx() ) << "Index wrong in FaceEdgeIter at end";
  EXPECT_FALSE(fe_it) << "Iterator invalid in FaceEdgeIter at end";
  EXPECT_TRUE( fe_it == fe_end )  << "End iterator for FaceEdgeIter not matching";

  Mesh::ConstFaceEdgeIter cfe_it  = mesh_.cfe_begin(mesh_.face_handle(1));
  Mesh::ConstFaceEdgeIter cfe_end = mesh_.cfe_end(mesh_.face_handle(1));

  EXPECT_EQ(4, cfe_it->idx() ) << "Index wrong in ConstFaceEdgeIter at initialization";
  EXPECT_TRUE(cfe_it) << "Iterator invalid in ConstFaceEdgeIter at initialization";
  --cfe_it;
  EXPECT_EQ(1, cfe_it->idx() ) << "Index wrong in ConstFaceEdgeIter at step 1";
  EXPECT_TRUE(cfe_it) << "Iterator invalid in ConstFaceEdgeIter at step 1";
  --cfe_it;
  EXPECT_EQ(3, cfe_it->idx() ) << "Index wrong in ConstFaceEdgeIter at step 2";
  EXPECT_TRUE(cfe_it) << "Iterator invalid in ConstFaceEdgeIter at step 2";
  --cfe_it;
  EXPECT_EQ(4, cfe_it->idx() ) << "Index wrong in ConstFaceEdgeIter at end";
  EXPECT_FALSE(cfe_it) << "Iterator invalid in ConstFaceEdgeIter at end";
  EXPECT_TRUE( cfe_it == cfe_end )  << "End iterator for ConstFaceEdgeIter not matching";

}
*/

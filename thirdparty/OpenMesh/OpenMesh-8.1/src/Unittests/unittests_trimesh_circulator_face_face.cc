#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>
#include <algorithm>

namespace {

class OpenMeshTrimeshCirculatorFaceFace : public OpenMeshBase {

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
 * Small FaceFaceIterator Test with holes in it
 */
TEST_F(OpenMeshTrimeshCirculatorFaceFace, FaceFaceIterWithHoles) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[5];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(3, 0, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(4, 1, 0));

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

  /* Test setup:
   *
   * 0 ------ 2 ------ 4
   *  \      / \      /
   *   \  0 /   \  2 /
   *    \  /  1  \  /
   *     1 ------- 3
   */


  Mesh::FaceFaceIter ff_it  = mesh_.ff_begin(mesh_.face_handle(1));
  Mesh::FaceFaceIter ff_end = mesh_.ff_end(mesh_.face_handle(1));

  EXPECT_EQ(2, ff_it->idx() ) << "Index wrong in FaceFaceIter at initialization";
  EXPECT_TRUE(ff_it.is_valid()) << "Iterator invalid in FaceFaceIter at initialization";
  ++ff_it;
  EXPECT_EQ(0, ff_it->idx() ) << "Index wrong in FaceFaceIter at step 1";
  EXPECT_TRUE(ff_it.is_valid()) << "Iterator invalid in FaceFaceIter at step 1";
  ++ff_it;
  EXPECT_EQ(2, ff_it->idx() ) << "Index wrong in FaceFaceIter at end";
  EXPECT_FALSE(ff_it.is_valid()) << "Iterator invalid in FaceFaceIter at end";
  EXPECT_TRUE( ff_it == ff_end )  << "End iterator for FaceFaceIter not matching";

  Mesh::ConstFaceFaceIter cff_it  = mesh_.cff_begin(mesh_.face_handle(1));
  Mesh::ConstFaceFaceIter cff_end = mesh_.cff_end(mesh_.face_handle(1));

  EXPECT_EQ(2, cff_it->idx() ) << "Index wrong in ConstFaceFaceIter at initialization";
  EXPECT_TRUE(cff_it.is_valid()) << "Iterator invalid in ConstFaceFaceIter at initialization";
  ++cff_it;
  EXPECT_EQ(0, cff_it->idx() ) << "Index wrong in ConstFaceFaceIter at step 1";
  EXPECT_TRUE(cff_it.is_valid()) << "Iterator invalid in ConstFaceFaceIter at step 1";
  ++cff_it;
  EXPECT_EQ(2, cff_it->idx() ) << "Index wrong in ConstFaceFaceIter at end";
  EXPECT_FALSE(cff_it.is_valid()) << "Iterator invalid in ConstFaceFaceIter at end";
  EXPECT_TRUE( cff_it == cff_end )  << "End iterator for ConstFaceFaceIter not matching";

}



/*
 * Small FaceFaceIterator Test with holes in it
 */
TEST_F(OpenMeshTrimeshCirculatorFaceFace, FaceFaceIterWithoutHoles) {

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


  Mesh::FaceFaceIter ff_it  = mesh_.ff_begin(mesh_.face_handle(1));
  Mesh::FaceFaceIter ff_end = mesh_.ff_end(mesh_.face_handle(1));

  EXPECT_EQ(2, ff_it->idx() ) << "Index wrong in FaceFaceIter at initialization";
  EXPECT_TRUE(ff_it.is_valid()) << "Iterator invalid in FaceFaceIter at initialization";
  ++ff_it;
  EXPECT_EQ(0, ff_it->idx() ) << "Index wrong in FaceFaceIter at step 1";
  EXPECT_TRUE(ff_it.is_valid()) << "Iterator invalid in FaceFaceIter at step 1";
  ++ff_it;
  EXPECT_EQ(3, ff_it->idx() ) << "Index wrong in FaceFaceIter at step 2";
  EXPECT_TRUE(ff_it.is_valid()) << "Iterator invalid in FaceFaceIter at step 2";
  ++ff_it;
  EXPECT_EQ(2, ff_it->idx() ) << "Index wrong in FaceFaceIter at end";
  EXPECT_FALSE(ff_it.is_valid()) << "Iterator invalid in FaceFaceIter at end";
  EXPECT_TRUE( ff_it == ff_end )  << "End iterator for FaceFaceIter not matching";

  Mesh::ConstFaceFaceIter cff_it  = mesh_.cff_begin(mesh_.face_handle(1));
  Mesh::ConstFaceFaceIter cff_end = mesh_.cff_end(mesh_.face_handle(1));

  EXPECT_EQ(2, cff_it->idx() ) << "Index wrong in ConstFaceFaceIter at initialization";
  EXPECT_TRUE(cff_it.is_valid()) << "Iterator invalid in ConstFaceFaceIter at initialization";
  ++cff_it;
  EXPECT_EQ(0, cff_it->idx() ) << "Index wrong in ConstFaceFaceIter at step 1";
  EXPECT_TRUE(cff_it.is_valid()) << "Iterator invalid in ConstFaceFaceIter at step 1";
  ++cff_it;
  EXPECT_EQ(3, cff_it->idx() ) << "Index wrong in ConstFaceFaceIter at step 2";
  EXPECT_TRUE(cff_it.is_valid()) << "Iterator invalid in ConstFaceFaceIter at step 2";
  ++cff_it;
  EXPECT_EQ(2, cff_it->idx() ) << "Index wrong in ConstFaceFaceIter at end";
  EXPECT_FALSE(cff_it.is_valid()) << "Iterator invalid in ConstFaceFaceIter at end";
  EXPECT_TRUE( cff_it == cff_end )  << "End iterator for ConstFaceFaceIter not matching";

}

/*
 * Test CW and CCW iterators
 */
TEST_F(OpenMeshTrimeshCirculatorFaceFace, CWAndCCWCheck) {

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


  int indices[4] = {2, 0, 3, 2};
  int rev_indices[4];
  std::reverse_copy(indices,indices+4,rev_indices);

  //CCW
  Mesh::FaceFaceCCWIter ff_ccwit  = mesh_.ff_ccwbegin(mesh_.face_handle(1));
  Mesh::FaceFaceCCWIter ff_ccwend = mesh_.ff_ccwend(mesh_.face_handle(1));
  size_t i = 0;
  for (;ff_ccwit != ff_ccwend; ++ff_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], ff_ccwit->idx()) << "Index wrong in FaceFaceCCWIter";
  }

  EXPECT_FALSE(ff_ccwit.is_valid()) << "Iterator invalid in FaceFaceCCWIter at end";
  EXPECT_TRUE( ff_ccwit == ff_ccwend )  << "End iterator for FaceFaceCCWIter not matching";

  //constant CCW
  Mesh::ConstFaceFaceCCWIter cff_ccwit  = mesh_.cff_ccwbegin(mesh_.face_handle(1));
  Mesh::ConstFaceFaceCCWIter cff_ccwend = mesh_.cff_ccwend(mesh_.face_handle(1));
  i = 0;
  for (;cff_ccwit != cff_ccwend; ++cff_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], cff_ccwit->idx()) << "Index wrong in ConstFaceFaceCCWIter";
  }

  EXPECT_FALSE(cff_ccwit.is_valid()) << "Iterator invalid in ConstFaceFaceCCWIter at end";
  EXPECT_TRUE( cff_ccwit == cff_ccwend )  << "End iterator for ConstFaceFaceCCWIter not matching";

  //CW
  Mesh::FaceFaceCWIter ff_cwit  = mesh_.ff_cwbegin(mesh_.face_handle(1));
  Mesh::FaceFaceCWIter ff_cwend = mesh_.ff_cwend(mesh_.face_handle(1));
  i = 0;
  for (;ff_cwit != ff_cwend; ++ff_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], ff_cwit->idx()) << "Index wrong in FaceFaceCWIter";
  }
  EXPECT_FALSE(ff_cwit.is_valid()) << "Iterator invalid in FaceFaceCWIter at end";
  EXPECT_TRUE( ff_cwit == ff_cwend )  << "End iterator for FaceFaceCWIter not matching";

  //constant CW
  Mesh::ConstFaceFaceCWIter cff_cwit  = mesh_.cff_cwbegin(mesh_.face_handle(1));
  Mesh::ConstFaceFaceCWIter cff_cwend = mesh_.cff_cwend(mesh_.face_handle(1));
  i = 0;
  for (;cff_cwit != cff_cwend; ++cff_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], cff_cwit->idx()) << "Index wrong in ConstFaceFaceCWIter";
  }
  EXPECT_FALSE(cff_cwit.is_valid()) << "Iterator invalid in ConstFaceFaceCWIter at end";
  EXPECT_TRUE( cff_cwit == cff_cwend )  << "End iterator for ConstFaceFaceCWIter not matching";

  /*
   * conversion properties:
   * a) cw_begin == CWIter(ccw_begin())
   * b) cw_iter->idx() == CCWIter(cw_iter)->idx() for valid iterators
   * c) --cw_iter == CWIter(++ccwIter) for valid iterators
   * d) cw_end == CWIter(ccw_end()) => --cw_end != CWIter(++ccw_end())   *
   */
  Mesh::FaceFaceCWIter ff_cwIter = mesh_.ff_cwbegin(mesh_.face_handle(1));
  // a)
  EXPECT_TRUE( ff_cwIter == Mesh::FaceFaceCWIter(mesh_.ff_ccwbegin(mesh_.face_handle(1))) ) << "ccw to cw conversion failed";
  EXPECT_TRUE( Mesh::FaceFaceCCWIter(ff_cwIter) == mesh_.ff_ccwbegin(mesh_.face_handle(1)) ) << "cw to ccw conversion failed";
  // b)
  EXPECT_EQ( ff_cwIter->idx(), Mesh::FaceFaceCCWIter(ff_cwIter)->idx()) << "iterators doesnt point on the same element";
  // c)
  ++ff_cwIter;
  ff_ccwend = mesh_.ff_ccwend(mesh_.face_handle(1));
  --ff_ccwend;
  EXPECT_EQ(ff_cwIter->idx(),ff_ccwend->idx()) << "iteratoes are not equal after inc/dec";
  // additional conversion check
  ff_ccwend = Mesh::FaceFaceCCWIter(ff_cwIter);
  EXPECT_EQ(ff_cwIter->idx(),ff_ccwend->idx())<< "iterators doesnt point on the same element";
  // d)
  ff_cwIter = Mesh::FaceFaceCWIter(mesh_.ff_ccwend(mesh_.face_handle(1)));
  EXPECT_FALSE(ff_cwIter.is_valid()) << "end iterator is not invalid";
  EXPECT_TRUE(Mesh::FaceFaceCCWIter(mesh_.ff_cwend(mesh_.face_handle(1))) ==  mesh_.ff_ccwend(mesh_.face_handle(1))) << "end iterators are not equal";


}



/*
 * Small FaceFaceIterator Test for getting handles and faces from the facefaceiterator
 */
TEST_F(OpenMeshTrimeshCirculatorFaceFace, FaceFaceIteratorHandleConversion) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));


  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  Mesh::FaceHandle fh1 = mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  /* Test setup:
   *
   *  1 -------- 2
   *  | f0  /    |
   *  |    / f1  |
   *  0 -------- 3
   */

  // Check setup
  EXPECT_EQ(4u , mesh_.n_vertices() ) << "Wrong number of vertices";
  EXPECT_EQ(2u , mesh_.n_faces() )    << "Wrong number of faces";


  Mesh::ConstFaceFaceIter face_iter = mesh_.cff_iter(fh1);


  // Get the face via the handle
  Mesh::FaceHandle faceHandle1 = *face_iter;

  EXPECT_EQ(1, faceHandle1.idx() ) << "Wrong face handle index when getting from iterator via handle";
}

/*
 * Test if the end iterator stays invalid after one lap
 */
//TEST_F(OpenMeshTrimeshCirculatorFaceFace, FaceFaceIterCheckInvalidationAtEnds) {
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
//  Mesh::FaceFaceIter endIter = mesh_.ff_end(fh0);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  ++endIter ;
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid after increment";
//
//  // Check if the end iterators becomes valid after decrement
//  endIter = mesh_.ff_end(fh0);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  --endIter;
//  EXPECT_TRUE(endIter.is_valid()) << "EndIter is invalid after decrement";
//  EXPECT_EQ(3,endIter->idx()) << "EndIter points on the wrong element";
//
//
//  // Check if the start iterator decrement is invalid
//  Mesh::FaceFaceIter startIter = mesh_.ff_begin(fh0);
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is not valid";
//  --startIter;
//  EXPECT_FALSE(startIter.is_valid()) << "StartIter decrement is not invalid";
//
//  // Check if the start iterator becomes valid
//  ++startIter;
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is invalid after re-incrementing";
//  EXPECT_EQ(startIter->idx(), mesh_.ff_begin(fh0)->idx()) << "StartIter points on the wrong element";
//
//}


}

#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshTrimeshCirculatorHalfedgeLoop : public OpenMeshBase {

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
 * Small HalfedgeLoop Test without holes in it
 */
TEST_F(OpenMeshTrimeshCirculatorHalfedgeLoop, HalfedgeLoopWithFace) {

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
   * edge x => halfedge x/x+1
   * i.e. edge 0 => halfedge 0/1
   *
   * 0 --4--- 2 ------ 4
   *  \      / \      /
   *   0  0 2   6  2 /
   *    \  /  1  \  /
   *     1 ---8--- 3
   *      \       /
   *       \  3  /
   *        \   /
   *         \ /
   *          5
   */

  //circle around face 1
  Mesh::HalfedgeLoopIter hl_it  = mesh_.hl_begin(mesh_.halfedge_handle(3));
  Mesh::HalfedgeLoopIter hl_end = mesh_.hl_end(mesh_.halfedge_handle(3));

  EXPECT_EQ(3, hl_it->idx() ) << "Index wrong in HalfedgeLoopIter at initialization";
  EXPECT_TRUE(hl_it.is_valid()) << "Iterator invalid in HalfedgeLoopIter at initialization";
  ++hl_it;
  EXPECT_EQ(6, hl_it->idx() ) << "Index wrong in HalfedgeLoopIter at step 1";
  EXPECT_TRUE(hl_it.is_valid()) << "Iterator invalid in HalfedgeLoopIter at step 1";
  ++hl_it;
  EXPECT_EQ(8, hl_it->idx() ) << "Index wrong in HalfedgeLoopIter at step 2";
  EXPECT_TRUE(hl_it.is_valid()) << "Iterator invalid in HalfedgeLoopIter at step 2";
  ++hl_it;
  EXPECT_EQ(3, hl_it->idx() ) << "Index wrong in HalfedgeLoopIter at step 3";
  EXPECT_FALSE( hl_it.is_valid() ) << "Iterator invalid in HalfedgeLoopIter at end";
  EXPECT_TRUE( hl_it == hl_end )  << "End iterator for HalfedgeLoopIter not matching";


  //check const iterators
  Mesh::HalfedgeLoopIter chl_it  = mesh_.chl_begin(mesh_.halfedge_handle(3));
  Mesh::HalfedgeLoopIter chl_end = mesh_.chl_end(mesh_.halfedge_handle(3));

  EXPECT_EQ(3, chl_it->idx() ) << "Index wrong in ConstHalfedgeLoopIter at initialization";
  EXPECT_TRUE(chl_it.is_valid()) << "Iterator invalid in ConstHalfedgeLoopIter at initialization";
  ++chl_it;
  EXPECT_EQ(6, chl_it->idx() ) << "Index wrong in ConstHalfedgeLoopIter at step 1";
  EXPECT_TRUE(chl_it.is_valid()) << "Iterator invalid in ConstHalfedgeLoopIter at step 1";
  ++chl_it;
  EXPECT_EQ(8, chl_it->idx() ) << "Index wrong in ConstHalfedgeLoopIter at step 2";
  EXPECT_TRUE(chl_it.is_valid()) << "Iterator invalid in ConstHalfedgeLoopIter at step 2";
  ++chl_it;
  EXPECT_EQ(3, chl_it->idx() ) << "Index wrong in ConstHalfedgeLoopIter at step 3";
  EXPECT_FALSE( chl_it.is_valid() ) << "Iterator invalid in ConstHalfedgeLoopIter at end";
  EXPECT_TRUE( chl_it == chl_end )  << "End iterator for ConstHalfedgeLoopIter not matching";

}


/*
 * Small HalfedgeLoop Test around a boundary
 */
TEST_F(OpenMeshTrimeshCirculatorHalfedgeLoop, HalfedgeLoopWithoutFace) {

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
   * H => hole (no face)
   * fx => face #x
   * edge 0 => halfedge 0/1
   *
   * 0 --4--- 2 -10--- 4
   *  \      / \      /
   *   0 f0 2   6 f2 8
   *    \  /  H  \  /
   *     1 ---16---3
   *      \       /
   *      12 f3 14
   *        \   /
   *         \ /
   *          5
   */

  //circle around the hole
  Mesh::HalfedgeLoopIter hl_it  = mesh_.hl_begin(mesh_.halfedge_handle(3));
  Mesh::HalfedgeLoopIter hl_end = mesh_.hl_end(mesh_.halfedge_handle(3));

  EXPECT_EQ(3, hl_it->idx() ) << "Index wrong in HalfedgeLoopIter at initialization";
  EXPECT_TRUE(hl_it.is_valid()) << "Iterator invalid in HalfedgeLoopIter at initialization";
  ++hl_it;
  EXPECT_EQ(17, hl_it->idx() ) << "Index wrong in HalfedgeLoopIter at step 1";
  EXPECT_TRUE(hl_it.is_valid()) << "Iterator invalid in HalfedgeLoopIter at step 1";
  ++hl_it;
  EXPECT_EQ(7, hl_it->idx() ) << "Index wrong in HalfedgeLoopIter at step 2";
  EXPECT_TRUE(hl_it.is_valid()) << "Iterator invalid in HalfedgeLoopIter at step 2";
  ++hl_it;
  EXPECT_EQ(3, hl_it->idx() ) << "Index wrong in HalfedgeLoopIter at step 3";
  EXPECT_FALSE( hl_it.is_valid() ) << "Iterator invalid in HalfedgeLoopIter at end";
  EXPECT_TRUE( hl_it == hl_end )  << "End iterator for HalfedgeLoopIter not matching";


  //check const iterators
  Mesh::HalfedgeLoopIter chl_it  = mesh_.chl_begin(mesh_.halfedge_handle(3));
  Mesh::HalfedgeLoopIter chl_end = mesh_.chl_end(mesh_.halfedge_handle(3));

  EXPECT_EQ(3, chl_it->idx() ) << "Index wrong in ConstHalfedgeLoopIter at initialization";
  EXPECT_TRUE(chl_it.is_valid()) << "Iterator invalid in ConstHalfedgeLoopIter at initialization";
  ++chl_it;
  EXPECT_EQ(17, chl_it->idx() ) << "Index wrong in ConstHalfedgeLoopIter at step 1";
  EXPECT_TRUE(chl_it.is_valid()) << "Iterator invalid in ConstHalfedgeLoopIter at step 1";
  ++chl_it;
  EXPECT_EQ(7, chl_it->idx() ) << "Index wrong in ConstHalfedgeLoopIter at step 2";
  EXPECT_TRUE(chl_it.is_valid()) << "Iterator invalid in ConstHalfedgeLoopIter at step 2";
  ++chl_it;
  EXPECT_EQ(3, chl_it->idx() ) << "Index wrong in ConstHalfedgeLoopIter at step 3";
  EXPECT_FALSE( chl_it.is_valid() ) << "Iterator invalid in ConstHalfedgeLoopIter at end";
  EXPECT_TRUE( chl_it == chl_end )  << "End iterator for ConstHalfedgeLoopIter not matching";

}

/*
 * Test CW and CCW iterators
 */
TEST_F(OpenMeshTrimeshCirculatorHalfedgeLoop, CWAndCCWCheck) {

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


  int indices[4] = {3, 6, 8, 3};
  int rev_indices[4];
  std::reverse_copy(indices,indices+4,rev_indices);

  //CCW
  Mesh::HalfedgeLoopCCWIter hl_ccwit  = mesh_.hl_ccwbegin(mesh_.halfedge_handle(3));
  Mesh::HalfedgeLoopCCWIter hl_ccwend = mesh_.hl_ccwend(mesh_.halfedge_handle(3));
  size_t i = 0;
  for (;hl_ccwit != hl_ccwend; ++hl_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], hl_ccwit->idx()) << "Index wrong in HalfedgeLoopCCWIter";
  }

  EXPECT_FALSE(hl_ccwit.is_valid()) << "Iterator invalid in HalfedgeLoopCCWIter at end";
  EXPECT_TRUE( hl_ccwit == hl_ccwend )  << "End iterator for HalfedgeLoopCCWIter not matching";

  //constant CCW
  Mesh::ConstHalfedgeLoopCCWIter chl_ccwit  = mesh_.chl_ccwbegin(mesh_.halfedge_handle(3));
  Mesh::ConstHalfedgeLoopCCWIter chl_ccwend = mesh_.chl_ccwend(mesh_.halfedge_handle(3));
  i = 0;
  for (;chl_ccwit != chl_ccwend; ++chl_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], chl_ccwit->idx()) << "Index wrong in ConstHalfedgeLoopCCWIter";
  }

  EXPECT_FALSE(chl_ccwit.is_valid()) << "Iterator invalid in ConstHalfedgeLoopCCWIter at end";
  EXPECT_TRUE( chl_ccwit == chl_ccwend )  << "End iterator for ConstHalfedgeLoopCCWIter not matching";

  //CW
  Mesh::HalfedgeLoopCWIter hl_cwit  = mesh_.hl_cwbegin(mesh_.halfedge_handle(3));
  Mesh::HalfedgeLoopCWIter hl_cwend = mesh_.hl_cwend(mesh_.halfedge_handle(3));
  i = 0;
  for (;hl_cwit != hl_cwend; ++hl_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], hl_cwit->idx()) << "Index wrong in HalfedgeLoopCWIter";
  }
  EXPECT_FALSE(hl_cwit.is_valid()) << "Iterator invalid in HalfedgeLoopCWIter at end";
  EXPECT_TRUE( hl_cwit == hl_cwend )  << "End iterator for HalfedgeLoopCWIter not matching";

  //constant CW
  Mesh::ConstHalfedgeLoopCWIter chl_cwit  = mesh_.chl_cwbegin(mesh_.halfedge_handle(3));
  Mesh::ConstHalfedgeLoopCWIter chl_cwend = mesh_.chl_cwend(mesh_.halfedge_handle(3));
  i = 0;
  for (;chl_cwit != chl_cwend; ++chl_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], chl_cwit->idx()) << "Index wrong in ConstHalfedgeLoopCWIter";
  }
  EXPECT_FALSE(chl_cwit.is_valid()) << "Iterator invalid in ConstHalfedgeLoopCWIter at end";
  EXPECT_TRUE( chl_cwit == chl_cwend )  << "End iterator for ConstHalfedgeLoopCWIter not matching";

  /*
   * conversion properties:
   * a) cw_begin == CWIter(ccw_begin())
   * b) cw_iter->idx() == CCWIter(cw_iter)->idx() for valid iterators
   * c) --cw_iter == CWIter(++ccwIter) for valid iterators
   * d) cw_end == CWIter(ccw_end()) => --cw_end != CWIter(++ccw_end())   *
   */
  Mesh::HalfedgeLoopCWIter hl_cwIter = mesh_.hl_cwbegin(mesh_.halfedge_handle(3));
  // a)
  EXPECT_TRUE( hl_cwIter == Mesh::HalfedgeLoopCWIter(mesh_.hl_ccwbegin(mesh_.halfedge_handle(3))) ) << "ccw to cw conversion failed";
  EXPECT_TRUE( Mesh::HalfedgeLoopCCWIter(hl_cwIter) == mesh_.hl_ccwbegin(mesh_.halfedge_handle(3)) ) << "cw to ccw conversion failed";
  // b)
  EXPECT_EQ( hl_cwIter->idx(), Mesh::HalfedgeLoopCCWIter(hl_cwIter)->idx()) << "iterators doesnt point on the same element";
  // c)
  ++hl_cwIter;
  hl_ccwend = mesh_.hl_ccwend(mesh_.halfedge_handle(3));
  --hl_ccwend;
  EXPECT_EQ(hl_cwIter->idx(),hl_ccwend->idx()) << "iteratoes are not equal after inc/dec";
  // additional conversion check
  hl_ccwend = Mesh::HalfedgeLoopCCWIter(hl_cwIter);
  EXPECT_EQ(hl_cwIter->idx(),hl_ccwend->idx())<< "iterators doesnt point on the same element";
  // d)
  hl_cwIter = Mesh::HalfedgeLoopCWIter(mesh_.hl_ccwend(mesh_.halfedge_handle(3)));
  EXPECT_FALSE(hl_cwIter.is_valid()) << "end iterator is not invalid";
  EXPECT_TRUE(Mesh::HalfedgeLoopCCWIter(mesh_.hl_cwend(mesh_.halfedge_handle(3))) ==  mesh_.hl_ccwend(mesh_.halfedge_handle(3))) << "end iterators are not equal";


}

}

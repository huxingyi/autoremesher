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

}

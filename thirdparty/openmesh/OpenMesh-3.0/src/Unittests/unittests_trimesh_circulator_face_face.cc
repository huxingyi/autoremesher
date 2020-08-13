#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

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
  Mesh::FaceHandle fh2 = mesh_.add_face(face_vhandles);

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
  const Mesh::Face& face1 = mesh_.face(faceHandle1);

  EXPECT_EQ(1, faceHandle1.idx() ) << "Wrong face handle index when getting from iterator via handle";
}
}

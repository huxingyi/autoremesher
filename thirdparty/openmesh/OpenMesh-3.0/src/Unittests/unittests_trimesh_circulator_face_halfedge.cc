#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

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
TEST_F(OpenMeshTrimeshCirculatorFaceHalfEdge, FaceHalfedgeIterWithoutHolesIncrement) {

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


  Mesh::FaceHalfedgeIter fh_it  = mesh_.fh_begin(mesh_.face_handle(1));
  Mesh::FaceHalfedgeIter fh_end = mesh_.fh_end(mesh_.face_handle(1));

  EXPECT_EQ(8, fh_it->idx() ) << "Index wrong in FaceHalfedgeIter at initialization";
  EXPECT_TRUE(fh_it.is_valid()) << "Iterator invalid in FaceHalfedgeIter at initialization";
  ++fh_it;
  EXPECT_EQ(3, fh_it->idx() ) << "Index wrong in FaceHalfedgeIter at step 1";
  EXPECT_TRUE(fh_it.is_valid()) << "Iterator invalid in FaceHalfedgeIter at step 1";
  ++fh_it;
  EXPECT_EQ(6, fh_it->idx() ) << "Index wrong in FaceHalfedgeIter at step 2";
  EXPECT_TRUE(fh_it.is_valid()) << "Iterator invalid in FaceHalfedgeIter at step 2";
  ++fh_it;
  EXPECT_EQ(8, fh_it->idx() ) << "Index wrong in FaceHalfedgeIter at end";
  EXPECT_FALSE(fh_it.is_valid()) << "Iterator invalid in FaceHalfedgeIter at end";
  EXPECT_TRUE( fh_it == fh_end )  << "End iterator for FaceHalfedgeIter not matching";

  Mesh::ConstFaceHalfedgeIter cfh_it  = mesh_.cfh_begin(mesh_.face_handle(1));
  Mesh::ConstFaceHalfedgeIter cfh_end = mesh_.cfh_end(mesh_.face_handle(1));

  EXPECT_EQ(8, cfh_it->idx() ) << "Index wrong in ConstFaceHalfedgeIter at initialization";
  EXPECT_TRUE(cfh_it.is_valid()) << "Iterator invalid in ConstFaceHalfedgeIter at initialization";
  ++cfh_it;
  EXPECT_EQ(3, cfh_it->idx() ) << "Index wrong in ConstFaceHalfedgeIter at step 1";
  EXPECT_TRUE(cfh_it.is_valid()) << "Iterator invalid in ConstFaceHalfedgeIter at step 1";
  ++cfh_it;
  EXPECT_EQ(6, cfh_it->idx() ) << "Index wrong in ConstFaceHalfedgeIter at step 2";
  EXPECT_TRUE(cfh_it.is_valid()) << "Iterator invalid in ConstFaceHalfedgeIter at step 2";
  ++cfh_it;
  EXPECT_EQ(8, cfh_it->idx() ) << "Index wrong in ConstFaceHalfedgeIter at end";
  EXPECT_FALSE(cfh_it.is_valid()) << "Iterator invalid in ConstFaceHalfedgeIter at end";
  EXPECT_TRUE( cfh_it == cfh_end )  << "End iterator for ConstFaceHalfedgeIter not matching";

}
}

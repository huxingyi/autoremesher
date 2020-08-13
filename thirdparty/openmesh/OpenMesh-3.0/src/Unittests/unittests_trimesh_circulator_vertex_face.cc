#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

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

  Mesh::VertexFaceIter vfa_it  = mesh_.vf_begin(vhandle[1]);

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
  EXPECT_EQ(3, vf_it->idx() ) << "Index wrong in VertexFaceIter at end";
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
  EXPECT_EQ(3, cvf_it->idx() ) << "Index wrong in ConstVertexFaceIter at end";
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
}

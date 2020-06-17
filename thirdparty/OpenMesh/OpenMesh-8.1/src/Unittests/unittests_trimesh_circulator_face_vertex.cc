#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshTrimeshCirculatorFaceVertex : public OpenMeshBase {

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
 * Small FaceVertexIterator Test
 */
TEST_F(OpenMeshTrimeshCirculatorFaceVertex, FaceVertexIterWithoutIncrement) {

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
  Mesh::FaceHandle fh0 = mesh_.add_face(face_vhandles);

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


  // Check face handle index
  EXPECT_EQ(0, fh0.idx() ) << "Index wrong in FaceVertexIter at initialization";

  // Iterate around vertex 1 at the middle (with holes in between)
  Mesh::FaceVertexIter fv_it  = mesh_.fv_begin(fh0);
  Mesh::FaceVertexIter fv_end = mesh_.fv_end(fh0);
  EXPECT_EQ(0, fv_it->idx() ) << "Index wrong in FaceVertexIter at initialization";
  EXPECT_TRUE(fv_it.is_valid()) << "Iterator invalid in FaceVertexIter at initialization";
  ++fv_it ;
  EXPECT_EQ(1, fv_it->idx() ) << "Index wrong in FaceVertexIter at step 1";
  EXPECT_TRUE(fv_it.is_valid()) << "Iterator invalid in FaceVertexIter at step 1";
  ++fv_it ;
  EXPECT_EQ(2, fv_it->idx() ) << "Index wrong in FaceVertexIter at step 2";
  EXPECT_TRUE(fv_it.is_valid()) << "Iterator invalid in FaceVertexIter at step 2";
  ++fv_it ;
  EXPECT_EQ(0, fv_it->idx() ) << "Index wrong in FaceVertexIter at step 3";
  EXPECT_FALSE(fv_it.is_valid()) << "Iterator invalid in FaceVertexIter at step 3";
  EXPECT_TRUE( fv_it == fv_end )  << "End iterator for FaceVertexIter not matching";

  // Iterate around vertex 1 at the middle (with holes in between) with const iterator
  Mesh::ConstFaceVertexIter cfv_it   = mesh_.cfv_begin(fh0);
  Mesh::ConstFaceVertexIter cfv_end  = mesh_.cfv_end(fh0);
  EXPECT_EQ(0, cfv_it->idx() ) << "Index wrong in ConstFaceVertexIter at initialization";
  EXPECT_TRUE(cfv_it.is_valid()) << "Iterator invalid in ConstFaceVertexIter at initialization";
  ++cfv_it ;
  EXPECT_EQ(1, cfv_it->idx() ) << "Index wrong in ConstFaceVertexIter at step 1";
  EXPECT_TRUE(cfv_it.is_valid()) << "Iterator invalid in ConstFaceVertexIter at step 1";
  ++cfv_it ;
  EXPECT_EQ(2, cfv_it->idx() ) << "Index wrong in ConstFaceVertexIter at step 2";
  EXPECT_TRUE(cfv_it.is_valid()) << "Iterator invalid in ConstFaceVertexIter at step 2";
  ++cfv_it ;
  EXPECT_EQ(0, cfv_it->idx() ) << "Index wrong in ConstFaceVertexIter at step 3";
  EXPECT_FALSE(cfv_it.is_valid()) << "Iterator invalid in ConstFaceVertexIter at step 3";
  EXPECT_TRUE( cfv_it == cfv_end )  << "End iterator for ConstFaceVertexIter not matching";
}


/*
 * Test if the end iterator stays invalid after one lap
 */
//TEST_F(OpenMeshTrimeshCirculatorFaceVertex, FaceVertexIterCheckInvalidationAtEnds) {
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
//  Mesh::FaceVertexIter endIter = mesh_.fv_end(fh0);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  ++endIter ;
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid after increment";
//
//  // Check if the end iterators becomes valid after decrement
//  endIter = mesh_.fv_end(fh0);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  --endIter;
//  EXPECT_TRUE(endIter.is_valid()) << "EndIter is invalid after decrement";
//  EXPECT_EQ(2,endIter->idx()) << "EndIter points on the wrong element";
//
//
//  // Check if the start iterator decrement is invalid
//  Mesh::FaceVertexIter startIter = mesh_.fv_begin(fh0);
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is not valid";
//  --startIter;
//  EXPECT_FALSE(startIter.is_valid()) << "StartIter decrement is not invalid";
//
//  // Check if the start iterator becomes valid
//  ++startIter;
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is invalid after re-incrementing";
//  EXPECT_EQ(startIter->idx(), mesh_.fv_begin(fh0)->idx()) << "StartIter points on the wrong element";
//
//}

/*
 * Test CW and CCW iterators
 */
TEST_F(OpenMeshTrimeshCirculatorFaceVertex, CWAndCCWCheck) {

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
  Mesh::FaceHandle fh0 = mesh_.add_face(face_vhandles);

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


  int indices[4] = {0, 1, 2, 0};
  int rev_indices[4];
  std::reverse_copy(indices,indices+4,rev_indices);

  //CCW
  Mesh::FaceVertexCCWIter fv_ccwit  = mesh_.fv_ccwbegin(fh0);
  Mesh::FaceVertexCCWIter fv_ccwend = mesh_.fv_ccwend(fh0);
  size_t i = 0;
  for (;fv_ccwit != fv_ccwend; ++fv_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], fv_ccwit->idx()) << "Index wrong in FaceVertexCCWIter";
  }

  EXPECT_FALSE(fv_ccwit.is_valid()) << "Iterator invalid in FaceVertexCCWIter at end";
  EXPECT_TRUE( fv_ccwit == fv_ccwend )  << "End iterator for FaceVertexCCWIter not matching";

  //constant CCW
  Mesh::ConstFaceVertexCCWIter cfv_ccwit  = mesh_.cfv_ccwbegin(fh0);
  Mesh::ConstFaceVertexCCWIter cfv_ccwend = mesh_.cfv_ccwend(fh0);
  i = 0;
  for (;cfv_ccwit != cfv_ccwend; ++cfv_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], cfv_ccwit->idx()) << "Index wrong in ConstFaceVertexCCWIter";
  }

  EXPECT_FALSE(cfv_ccwit.is_valid()) << "Iterator invalid in ConstFaceVertexCCWIter at end";
  EXPECT_TRUE( cfv_ccwit == cfv_ccwend )  << "End iterator for ConstFaceVertexCCWIter not matching";

  //CW
  Mesh::FaceVertexCWIter fv_cwit  = mesh_.fv_cwbegin(fh0);
  Mesh::FaceVertexCWIter fv_cwend = mesh_.fv_cwend(fh0);
  i = 0;
  for (;fv_cwit != fv_cwend; ++fv_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], fv_cwit->idx()) << "Index wrong in FaceVertexCWIter";
  }
  EXPECT_FALSE(fv_cwit.is_valid()) << "Iterator invalid in FaceVertexCWIter at end";
  EXPECT_TRUE( fv_cwit == fv_cwend )  << "End iterator for FaceVertexCWIter not matching";

  //constant CW
  Mesh::ConstFaceVertexCWIter cfv_cwit  = mesh_.cfv_cwbegin(fh0);
  Mesh::ConstFaceVertexCWIter cfv_cwend = mesh_.cfv_cwend(fh0);
  i = 0;
  for (;cfv_cwit != cfv_cwend; ++cfv_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], cfv_cwit->idx()) << "Index wrong in ConstFaceVertexCWIter";
  }
  EXPECT_FALSE(cfv_cwit.is_valid()) << "Iterator invalid in ConstFaceVertexCWIter at end";
  EXPECT_TRUE( cfv_cwit == cfv_cwend )  << "End iterator for ConstFaceVertexCWIter not matching";

  /*
   * conversion properties:
   * a) cw_begin == CWIter(ccw_begin())
   * b) cw_iter->idx() == CCWIter(cw_iter)->idx() for valid iterators
   * c) --cw_iter == CWIter(++ccwIter) for valid iterators
   * d) cw_end == CWIter(ccw_end()) => --cw_end != CWIter(++ccw_end())   *
   */
  Mesh::FaceVertexCWIter fv_cwIter = mesh_.fv_cwbegin(mesh_.face_handle(1));
  // a)
  EXPECT_TRUE( fv_cwIter == Mesh::FaceVertexCWIter(mesh_.fv_ccwbegin(mesh_.face_handle(1))) ) << "ccw to cw conversion failed";
  EXPECT_TRUE( Mesh::FaceVertexCCWIter(fv_cwIter) == mesh_.fv_ccwbegin(mesh_.face_handle(1)) ) << "cw to ccw conversion failed";
  // b)
  EXPECT_EQ( fv_cwIter->idx(), Mesh::FaceVertexCCWIter(fv_cwIter)->idx()) << "iterators doesnt point on the same element";
  // c)
  ++fv_cwIter;
  fv_ccwend = mesh_.fv_ccwend(mesh_.face_handle(1));
  --fv_ccwend;
  EXPECT_EQ(fv_cwIter->idx(),fv_ccwend->idx()) << "iteratoes are not equal after inc/dec";
  // additional conversion check
  fv_ccwend = Mesh::FaceVertexCCWIter(fv_cwIter);
  EXPECT_EQ(fv_cwIter->idx(),fv_ccwend->idx())<< "iterators doesnt point on the same element";
  // d)
  fv_cwIter = Mesh::FaceVertexCWIter(mesh_.fv_ccwend(mesh_.face_handle(1)));
  EXPECT_FALSE(fv_cwIter.is_valid()) << "end iterator is not invalid";
  EXPECT_TRUE(Mesh::FaceVertexCCWIter(mesh_.fv_cwend(mesh_.face_handle(1))) ==  mesh_.fv_ccwend(mesh_.face_handle(1))) << "end iterators are not equal";


}


}

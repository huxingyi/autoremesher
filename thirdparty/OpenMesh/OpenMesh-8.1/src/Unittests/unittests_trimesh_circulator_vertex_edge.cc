#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshTrimeshCirculatorVertexEdge : public OpenMeshBase {

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
 * Small VertexEdgeIterator Test without holes in it
 */
TEST_F(OpenMeshTrimeshCirculatorVertexEdge, VertexEdgeIterWithoutHolesIncrement) {

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
  Mesh::VertexEdgeIter ve_it  = mesh_.ve_begin(vhandle[1]);
  Mesh::VertexEdgeIter ve_end = mesh_.ve_end(vhandle[1]);
  EXPECT_EQ(5, ve_it->idx() ) << "Index wrong in VertexEdgeIter at initialization";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at initialization";
  ++ve_it ;
  EXPECT_EQ(3, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 1";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at step 1";
  ++ve_it ;
  EXPECT_EQ(0, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 2";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at step 2";
  ++ve_it ;
  EXPECT_EQ(1, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 3";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at step 3";
  ++ve_it ;
  EXPECT_EQ(5, ve_it->idx() ) << "Index wrong in VertexEdgeIter at end";
  EXPECT_FALSE(ve_it.is_valid()) << "Iterator not invalid in VertexEdgeIter at end";
  EXPECT_TRUE( ve_it == ve_end )  << "End iterator for VertexEdgeIter not matching";

  // Iterate around vertex 1 at the middle (with holes in between) with const iterator
  Mesh::ConstVertexEdgeIter cve_it  = mesh_.cve_begin(vhandle[1]);
  Mesh::ConstVertexEdgeIter cve_end = mesh_.cve_end(vhandle[1]);
  EXPECT_EQ(5, cve_it->idx() ) << "Index wrong in ConstVertexEdgeIter at initialization";
  EXPECT_TRUE(cve_it.is_valid()) << "Iterator invalid in ConstVertexEdgeIter at initialization";
  ++cve_it ;
  EXPECT_EQ(3, cve_it->idx() ) << "Index wrong in ConstVertexEdgeIter at step 1";
  EXPECT_TRUE(cve_it.is_valid()) << "Iterator invalid in ConstVertexEdgeIter at step 1";
  ++cve_it ;
  EXPECT_EQ(0, cve_it->idx() ) << "Index wrong in ConstVertexEdgeIter at step 2";
  EXPECT_TRUE(cve_it.is_valid()) << "Iterator invalid in ConstVertexEdgeIter at step 2";
  ++cve_it ;
  EXPECT_EQ(1, cve_it->idx() ) << "Index wrong in ConstVertexEdgeIter at step 3";
  EXPECT_TRUE(cve_it.is_valid()) << "Iterator invalid in ConstVertexEdgeIter at step 3";
  ++cve_it ;
  EXPECT_EQ(5, cve_it->idx() ) << "Index wrong in ConstVertexEdgeIter at end";
  EXPECT_FALSE(cve_it.is_valid()) << "Iterator not invalid in VertexEdgeIter at end";
  EXPECT_TRUE( cve_it == cve_end )  << "End iterator for ConstVertexEdgeIter not matching";
}

/*
 * Small VertexEdgeIterator Test without holes in it
 */
TEST_F(OpenMeshTrimeshCirculatorVertexEdge, VertexEdgeIterBoundaryIncrement) {

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
  Mesh::VertexEdgeIter ve_it  = mesh_.ve_begin(vhandle[2]);
  Mesh::VertexEdgeIter ve_end = mesh_.ve_end(vhandle[2]);
  EXPECT_EQ(7, ve_it->idx() ) << "Index wrong in VertexEdgeIter at initialization";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at initialization";
  ++ve_it ;
  EXPECT_EQ(1, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 1";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at step 1";
  ++ve_it ;
  EXPECT_EQ(2, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 2";
  EXPECT_TRUE(ve_it.is_valid()) << "Iterator invalid in VertexEdgeIter at step 2";
  ++ve_it ;
  EXPECT_EQ(7, ve_it->idx() ) << "Index wrong in VertexEdgeIter at step 3";
  EXPECT_FALSE(ve_it.is_valid()) << "Iterator not invalid in VertexEdgeIter at end";
  EXPECT_TRUE( ve_it == ve_end )  << "End iterator for VertexEdgeIter not matching";

}


/*
 * Test if the end iterator stays invalid after one lap
 * DISABLED as long as the normal iterators using old behavior
 */
//TEST_F(OpenMeshTrimeshCirculatorVertexEdge, VertexEdgeIterCheckInvalidationAtEnds) {
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
//  Mesh::VertexEdgeIter endIter = mesh_.ve_end(vhandle[1]);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  ++endIter ;
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid after increment";
//
//  // Check if the end iterators becomes valid after decrement
//  endIter = mesh_.ve_end(vhandle[1]);
//  EXPECT_FALSE(endIter.is_valid()) << "EndIter is not invalid";
//  --endIter;
//  EXPECT_TRUE(endIter.is_valid()) << "EndIter is invalid after decrement";
//  EXPECT_EQ(1,endIter->idx()) << "EndIter points on the wrong element";
//
//
//  // Check if the start iterator decrement is invalid
//  Mesh::VertexEdgeIter startIter = mesh_.ve_begin(vhandle[1]);
//  EXPECT_TRUE(startIter.is_valid()) << "StartIter is not valid";
//  --startIter;
//  EXPECT_FALSE(startIter.is_valid()) << "StartIter decrement is not invalid";
//
//  // Check if the start iterator becomes valid
//  ++startIter;
//  EXPECT_TRUE(startIter.is_valid()) << "StarIter is invalid after re-incrementing";
//  EXPECT_EQ(startIter->idx(), mesh_.ve_begin(vhandle[1])->idx()) << "StartIter points on the wrong element";
//
//}

/*
 * Test CW and CCW iterators
 */
TEST_F(OpenMeshTrimeshCirculatorVertexEdge, CWAndCCWCheck) {

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


  int indices[5] = {5, 1, 0, 3, 5};
  int rev_indices[5];
  std::reverse_copy(indices,indices+5,rev_indices);

  Mesh::VertexHandle vh = vhandle[1];

  //CCW
  Mesh::VertexEdgeCCWIter ve_ccwit  = mesh_.ve_ccwbegin(vh);
  Mesh::VertexEdgeCCWIter ve_ccwend = mesh_.ve_ccwend(vh);
  size_t i = 0;
  for (;ve_ccwit != ve_ccwend; ++ve_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], ve_ccwit->idx()) << "Index wrong in VertexEdgeCCWIter";
  }

  EXPECT_FALSE(ve_ccwit.is_valid()) << "Iterator invalid in VertexEdgeCCWIter at end";
  EXPECT_TRUE( ve_ccwit == ve_ccwend )  << "End iterator for VertexEdgeCCWIter not matching";

  //constant CCW
  Mesh::ConstVertexEdgeCCWIter cve_ccwit  = mesh_.cve_ccwbegin(vh);
  Mesh::ConstVertexEdgeCCWIter cve_ccwend = mesh_.cve_ccwend(vh);
  i = 0;
  for (;cve_ccwit != cve_ccwend; ++cve_ccwit, ++i)
  {
    EXPECT_EQ(indices[i], cve_ccwit->idx()) << "Index wrong in ConstVertexEdgeCCWIter";
  }

  EXPECT_FALSE(cve_ccwit.is_valid()) << "Iterator invalid in ConstVertexEdgeCCWIter at end";
  EXPECT_TRUE( cve_ccwit == cve_ccwend )  << "End iterator for ConstVertexEdgeCCWIter not matching";

  //CW
  Mesh::VertexEdgeCWIter ve_cwit  = mesh_.ve_cwbegin(vh);
  Mesh::VertexEdgeCWIter ve_cwend = mesh_.ve_cwend(vh);
  i = 0;
  for (;ve_cwit != ve_cwend; ++ve_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], ve_cwit->idx()) << "Index wrong in VertexEdgeCWIter";
  }
  EXPECT_FALSE(ve_cwit.is_valid()) << "Iterator invalid in VertexEdgeCWIter at end";
  EXPECT_TRUE( ve_cwit == ve_cwend )  << "End iterator for VertexEdgeCWIter not matching";

  //constant CW
  Mesh::ConstVertexEdgeCWIter cve_cwit  = mesh_.cve_cwbegin(vh);
  Mesh::ConstVertexEdgeCWIter cve_cwend = mesh_.cve_cwend(vh);
  i = 0;
  for (;cve_cwit != cve_cwend; ++cve_cwit, ++i)
  {
    EXPECT_EQ(rev_indices[i], cve_cwit->idx()) << "Index wrong in ConstVertexEdgeCWIter";
  }
  EXPECT_FALSE(cve_cwit.is_valid()) << "Iterator invalid in ConstVertexEdgeCWIter at end";
  EXPECT_TRUE( cve_cwit == cve_cwend )  << "End iterator for ConstVertexEdgeCWIter not matching";

  /*
   * conversion properties:
   * a) cw_begin == CWIter(ccw_begin())
   * b) cw_iter->idx() == CCWIter(cw_iter)->idx() for valid iterators
   * c) --cw_iter == CWIter(++ccwIter) for valid iterators
   * d) cw_end == CWIter(ccw_end()) => --cw_end != CWIter(++ccw_end())   *
   */
  Mesh::VertexEdgeCWIter ve_cwIter = mesh_.ve_cwbegin(vh);
  // a)
  EXPECT_TRUE( ve_cwIter == Mesh::VertexEdgeCWIter(mesh_.ve_ccwbegin(vh)) ) << "ccw to cw conversion failed";
  EXPECT_TRUE( Mesh::VertexEdgeCCWIter(ve_cwIter) == mesh_.ve_ccwbegin(vh) ) << "cw to ccw conversion failed";
  // b)
  EXPECT_EQ( ve_cwIter->idx(), Mesh::VertexEdgeCCWIter(ve_cwIter)->idx()) << "iterators doesnt point on the same element";
  // c)
  ++ve_cwIter;
  ve_ccwend = mesh_.ve_ccwend(vh);
  --ve_ccwend;
  EXPECT_EQ(ve_cwIter->idx(),ve_ccwend->idx()) << "iteratoes are not equal after inc/dec";
  // additional conversion check
  ve_ccwend = Mesh::VertexEdgeCCWIter(ve_cwIter);
  EXPECT_EQ(ve_cwIter->idx(),ve_ccwend->idx())<< "iterators doesnt point on the same element";
  // d)
  ve_cwIter = Mesh::VertexEdgeCWIter(mesh_.ve_ccwend(vh));
  EXPECT_FALSE(ve_cwIter.is_valid()) << "end iterator is not invalid";
  EXPECT_TRUE(Mesh::VertexEdgeCCWIter(mesh_.ve_cwend(vh)) ==  mesh_.ve_ccwend(vh)) << "end iterators are not equal";


}

}

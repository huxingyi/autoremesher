#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshCollapse : public OpenMeshBase {

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
 * Collapsing a quad splitted with center vertex
 */
TEST_F(OpenMeshCollapse, CollapseQuadWithCenter) {

  mesh_.clear();


  // 0--------1
  // |\      /|
  // | \    / | 
  // |  \  /  |
  // |    2   |
  // |  /  \  |
  // | /    \ |
  // 3--------4

  // Add some vertices
  Mesh::VertexHandle vhandle[5];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(2, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0, 2, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(2, 2, 0));

  // Add four faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_face_status();


  // Get the halfedge
  Mesh::HalfedgeHandle v2v1 = mesh_.find_halfedge(vhandle[2], vhandle[1]);

  EXPECT_TRUE( v2v1.is_valid() ) << "Invalid halfedge returned although it shoud exist";
  EXPECT_TRUE( mesh_.is_collapse_ok(v2v1) ) << "Collapse retuned illegal althoug legal";

  // Execute it as a crash test
  mesh_.collapse(v2v1);
}




/*
 * Collapsing a tetrahedron
 */
TEST_F(OpenMeshCollapse, CollapseTetrahedronComplex) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));

  // Add four faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  mesh_.add_face(face_vhandles);

  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_face_status();

  Mesh::HalfedgeHandle v0v1 = mesh_.halfedge_handle(0);
  Mesh::HalfedgeHandle v1v0 = mesh_.opposite_halfedge_handle(v0v1);

  Mesh::HalfedgeHandle v1vL = mesh_.next_halfedge_handle(v0v1);
  Mesh::HalfedgeHandle vLv1 = mesh_.opposite_halfedge_handle(v1vL);
  Mesh::HalfedgeHandle vLv0 = mesh_.next_halfedge_handle(v1vL);
  Mesh::HalfedgeHandle v0vL = mesh_.opposite_halfedge_handle(vLv0);

  Mesh::HalfedgeHandle vLvR = mesh_.next_halfedge_handle(v0vL);
  Mesh::HalfedgeHandle vRvL = mesh_.opposite_halfedge_handle(vLvR);

  Mesh::HalfedgeHandle v0vR = mesh_.next_halfedge_handle(v1v0);
  Mesh::HalfedgeHandle vRv0 = mesh_.opposite_halfedge_handle(v0vR);
  Mesh::HalfedgeHandle vRv1 = mesh_.next_halfedge_handle(v0vR);
  Mesh::HalfedgeHandle v1vR = mesh_.opposite_halfedge_handle(vRv1);



  Mesh::VertexHandle v0     = mesh_.from_vertex_handle(v0v1);
  Mesh::VertexHandle v1     = mesh_.to_vertex_handle(v0v1);
  Mesh::VertexHandle vL     = mesh_.to_vertex_handle(mesh_.next_halfedge_handle(v0v1));
  Mesh::VertexHandle vR     = mesh_.to_vertex_handle(mesh_.next_halfedge_handle(v1v0));

  // ===================================================================
  // Check preconditions
  // ===================================================================

  EXPECT_TRUE( mesh_.is_collapse_ok(v0v1) ) << "Collapse not ok for halfedge 0";
  EXPECT_TRUE( mesh_.is_collapse_ok(v1v0) ) << "Collapse not ok for opposite of halfedge 0";

  // Test the Vertex indices
  EXPECT_EQ(0, v0.idx() ) << "Index wrong for from vertex of collapse halfedge";
  EXPECT_EQ(1, v1.idx() ) << "Index wrong for to vertex of collapse halfedge";
  EXPECT_EQ(2, vL.idx() ) << "Index wrong for left vertex of collapse halfedge";
  EXPECT_EQ(3, vR.idx() ) << "Index wrong for right vertex of collapse halfedge";

  // Check the halfedges
  EXPECT_EQ(0, v0v1.idx() ) << "Index wrong for collapse halfedge";
  EXPECT_EQ(1, v1v0.idx() ) << "Index wrong for opposite collapse halfedge";

  EXPECT_EQ(2  , v1vL.idx() ) << "Index wrong for v1vL halfedge";
  EXPECT_EQ(3  , vLv1.idx() ) << "Index wrong for vLv1 halfedge";
  EXPECT_EQ(4  , vLv0.idx() ) << "Index wrong for vLv0 halfedge";
  EXPECT_EQ(5  , v0vL.idx() ) << "Index wrong for v0vL halfedge";

  EXPECT_EQ(6  , vLvR.idx() ) << "Index wrong for vLvR halfedge";
  EXPECT_EQ(7  , vRvL.idx() ) << "Index wrong for vRvL halfedge";

  EXPECT_EQ(8  , vRv0.idx() ) << "Index wrong for vRv0 halfedge";
  EXPECT_EQ(9  , v0vR.idx() ) << "Index wrong for v0vR halfedge";

  EXPECT_EQ(10 , v1vR.idx() ) << "Index wrong for v1vR halfedge";
  EXPECT_EQ(11 , vRv1.idx() ) << "Index wrong for vRv1 halfedge";

  // ===================================================================
  // Execute collapse
  // ===================================================================

  mesh_.collapse(v0v1);

  // ===================================================================
  // Check configuration afterwards
  // ===================================================================

  /** Now the configuration should look like this:
   *  The numbers at the side denote the halfedges
   *           1
   *          / \
   *         /   \
   *       //    \\
   *      3/2    11\10
   *      //        \\
   *     /    6-->   \
   *    2 ----------- 3
   *        <--7
   *
   */


  EXPECT_EQ(4u , mesh_.n_faces() ) << "Wrong number of faces (garbage collection not executed!)";

  // Check if the right vertices got deleted
  EXPECT_TRUE( mesh_.status(mesh_.face_handle(0)).deleted() ) << "Face 0 not deleted";
  EXPECT_FALSE( mesh_.status(mesh_.face_handle(1)).deleted() ) << "Face 1 deleted";
  EXPECT_FALSE( mesh_.status(mesh_.face_handle(2)).deleted() ) << "Face 2 deleted";
  EXPECT_TRUE( mesh_.status(mesh_.face_handle(3)).deleted() ) << "Face 3 not deleted";

  // Check the vertices of the two remaining faces
  Mesh::FaceHandle fh_1 = mesh_.face_handle(1);
  Mesh::FaceHandle fh_2 = mesh_.face_handle(2);

  Mesh::FaceVertexIter fv_it =  mesh_.fv_begin(fh_1);

  EXPECT_EQ(1 , fv_it->idx() ) << "Index wrong for Vertex 1 of face 1";
  ++fv_it;
  EXPECT_EQ(2 , fv_it->idx() ) << "Index wrong for Vertex 2 of face 1";
  ++fv_it;
  EXPECT_EQ(3 , fv_it->idx() ) << "Index wrong for Vertex 3 of face 1";

  fv_it =  mesh_.fv_begin(fh_2);
  EXPECT_EQ(2 , fv_it->idx() ) << "Index wrong for Vertex 1 of face 2";
  ++fv_it;
  EXPECT_EQ(1 , fv_it->idx() ) << "Index wrong for Vertex 2 of face 2";
  ++fv_it;
  EXPECT_EQ(3 , fv_it->idx() ) << "Index wrong for Vertex 3 of face 2";

  // Get the first halfedge of face 1
  Mesh::HalfedgeHandle fh_1_he = mesh_.halfedge_handle(fh_1);

  EXPECT_EQ(11 , fh_1_he.idx() ) << "Index wrong for first halfedge of face 1";
  EXPECT_EQ(1  , mesh_.to_vertex_handle(fh_1_he).idx() ) << "First halfedge inside face 1 pointing to wrong vertex";

  Mesh::HalfedgeHandle next = mesh_.next_halfedge_handle(fh_1_he);
  EXPECT_EQ(2 , next.idx() ) << "Index wrong for second halfedge inside face 1 ";
  EXPECT_EQ(2 , mesh_.to_vertex_handle(next).idx() ) << "second halfedge inside face 1 pointing to wrong vertex ";

  next = mesh_.next_halfedge_handle(next);
  EXPECT_EQ(6 , next.idx() ) << "Index wrong for third halfedge inside face 1 ";
  EXPECT_EQ(3 , mesh_.to_vertex_handle(next).idx() ) << "Third halfedge inside face 1 pointing to wrong vertex ";

  // Get the first halfedge of face 2
  Mesh::HalfedgeHandle fh_2_he = mesh_.halfedge_handle(fh_2);

  EXPECT_EQ(7 , fh_2_he.idx() ) << "Index wrong for first halfedge of face 2";
  EXPECT_EQ(2  , mesh_.to_vertex_handle(fh_2_he).idx() ) << "First halfedge inside face 2 pointing to wrong vertex";

  next = mesh_.next_halfedge_handle(fh_2_he);
  EXPECT_EQ(3 , next.idx() ) << "Index wrong for second halfedge inside face 2";
  EXPECT_EQ(1 , mesh_.to_vertex_handle(next).idx() ) << "second halfedge inside face 2 pointing to wrong vertex ";

  next = mesh_.next_halfedge_handle(next);
  EXPECT_EQ(10 , next.idx() ) << "Index wrong for third halfedge inside face 2";
  EXPECT_EQ(3 , mesh_.to_vertex_handle(next).idx() ) << "Third halfedge inside face 2 pointing to wrong vertex ";

  // Vertex 1 outgoing
  Mesh::VertexOHalfedgeIter voh_it = mesh_.voh_begin(mesh_.vertex_handle(1));
  EXPECT_EQ(10 , voh_it->idx() ) << "Index wrong for first outgoing halfedge of vertex 1";
  ++voh_it;
  EXPECT_EQ(2  , voh_it->idx() ) << "Index wrong for second outgoing halfedge of vertex 1";
  ++voh_it;
  EXPECT_EQ(10 , voh_it->idx() ) << "Index wrong for third(one lap) outgoing halfedge of vertex 1";

  // Vertex 2 outgoing
  voh_it = mesh_.voh_begin(mesh_.vertex_handle(2));
  EXPECT_EQ(3 , voh_it->idx() ) << "Index wrong for first outgoing halfedge of vertex 2";
  ++voh_it;
  EXPECT_EQ(6 , voh_it->idx() ) << "Index wrong for second outgoing halfedge of vertex 2";
  ++voh_it;
  EXPECT_EQ(3 , voh_it->idx() ) << "Index wrong for third(one lap) outgoing halfedge of vertex 2";

  // Vertex 3 outgoing
  voh_it = mesh_.voh_begin(mesh_.vertex_handle(3));
  EXPECT_EQ(11 , voh_it->idx() ) << "Index wrong for first outgoing halfedge of vertex 3";
  ++voh_it;
  EXPECT_EQ(7  , voh_it->idx() ) << "Index wrong for second outgoing halfedge of vertex 3";
  ++voh_it;
  EXPECT_EQ(11 , voh_it->idx() ) << "Index wrong for third(one lap) outgoing halfedge of vertex 3";

  // ===================================================================
  // Cleanup
  // ===================================================================
  mesh_.garbage_collection();

  // ===================================================================
  // Check configuration afterwards
  // ===================================================================

  /** Now the configuration should look like this:
     *  The numbers at the side denote the halfedges
     *           0
     *          / \
     *         /   \
     *       //    \\
     *      4/5     0\1
     *      //        \\
     *     /    3-->   \
     *    2 ----------- 1
     *        <--2
     *
     */

  EXPECT_EQ(2u , mesh_.n_faces() ) << "Wrong number of faces (garbage collection executed!)";

  // Check the vertices of the two remaining faces
  Mesh::FaceHandle fh_0 = mesh_.face_handle(0);
  fh_1 = mesh_.face_handle(1);

  fv_it =  mesh_.fv_begin(fh_0);

  EXPECT_EQ(2 , fv_it->idx() ) << "Index wrong for Vertex 1 of face 0 after garbage collection";
  ++fv_it;
  EXPECT_EQ(1 , fv_it->idx() ) << "Index wrong for Vertex 2 of face 0 after garbage collection";
  ++fv_it;
  EXPECT_EQ(0 , fv_it->idx() ) << "Index wrong for Vertex 3 of face 0 after garbage collection";

  fv_it =  mesh_.fv_begin(fh_1);
  EXPECT_EQ(1 , fv_it->idx() ) << "Index wrong for Vertex 1 of face 1 after garbage collection";
  ++fv_it;
  EXPECT_EQ(2 , fv_it->idx() ) << "Index wrong for Vertex 2 of face 1 after garbage collection";
  ++fv_it;
  EXPECT_EQ(0 , fv_it->idx() ) << "Index wrong for Vertex 3 of face 1 after garbage collection";

  // Get the first halfedge of face 1
  Mesh::HalfedgeHandle fh_0_he = mesh_.halfedge_handle(fh_0);

  EXPECT_EQ(5 , fh_0_he.idx() ) << "Index wrong for first halfedge of face 0";
  EXPECT_EQ(2  , mesh_.to_vertex_handle(fh_0_he).idx() ) << "First halfedge inside face 0 pointing to wrong vertex";

  next = mesh_.next_halfedge_handle(fh_0_he);
  EXPECT_EQ(3 , next.idx() ) << "Index wrong for second halfedge inside face 0 ";
  EXPECT_EQ(1 , mesh_.to_vertex_handle(next).idx() ) << "second halfedge inside face 0 pointing to wrong vertex ";

  next = mesh_.next_halfedge_handle(next);
  EXPECT_EQ(0 , next.idx() ) << "Index wrong for third halfedge inside face 0 ";
  EXPECT_EQ(0 , mesh_.to_vertex_handle(next).idx() ) << "Third halfedge inside face 0 pointing to wrong vertex ";

  // Get the first halfedge of face 1
  fh_1_he = mesh_.halfedge_handle(fh_1);

  EXPECT_EQ(1 , fh_1_he.idx() ) << "Index wrong for first halfedge of face 1";
  EXPECT_EQ(1  , mesh_.to_vertex_handle(fh_1_he).idx() ) << "First halfedge inside face 1 pointing to wrong vertex";

  next = mesh_.next_halfedge_handle(fh_1_he);
  EXPECT_EQ(2 , next.idx() ) << "Index wrong for second halfedge inside face 1 ";
  EXPECT_EQ(2 , mesh_.to_vertex_handle(next).idx() ) << "second halfedge inside face 1 pointing to wrong vertex ";

  next = mesh_.next_halfedge_handle(next);
  EXPECT_EQ(4 , next.idx() ) << "Index wrong for third halfedge inside face 1 ";
  EXPECT_EQ(0 , mesh_.to_vertex_handle(next).idx() ) << "Third halfedge inside face 1 pointing to wrong vertex ";


  // Vertex 0 outgoing
  voh_it = mesh_.voh_begin(mesh_.vertex_handle(0));
  EXPECT_EQ(1 , voh_it->idx() ) << "Index wrong for first outgoing halfedge of vertex 0";
  ++voh_it;
  EXPECT_EQ(5 , voh_it->idx() ) << "Index wrong for second outgoing halfedge of vertex 0";
  ++voh_it;
  EXPECT_EQ(1 , voh_it->idx() ) << "Index wrong for third(one lap) outgoing halfedge of vertex 0";

  // Vertex 1 outgoing
  voh_it = mesh_.voh_begin(mesh_.vertex_handle(1));
  EXPECT_EQ(0 , voh_it->idx() ) << "Index wrong for first outgoing halfedge of vertex 1";
  ++voh_it;
  EXPECT_EQ(2 , voh_it->idx() ) << "Index wrong for second outgoing halfedge of vertex 1";
  ++voh_it;
  EXPECT_EQ(0 , voh_it->idx() ) << "Index wrong for third(one lap) outgoing halfedge of vertex 1";

  // Vertex 2 outgoing
  voh_it = mesh_.voh_begin(mesh_.vertex_handle(2));
  EXPECT_EQ(3 , voh_it->idx() ) << "Index wrong for first outgoing halfedge of vertex 2";
  ++voh_it;
  EXPECT_EQ(4 , voh_it->idx() ) << "Index wrong for second outgoing halfedge of vertex 2";
  ++voh_it;
  EXPECT_EQ(3 , voh_it->idx() ) << "Index wrong for third(one lap) outgoing halfedge of vertex 2";

  EXPECT_FALSE( mesh_.is_collapse_ok(mesh_.halfedge_handle(0)) ) << "Collapse should be not ok for halfedge 0";
  EXPECT_FALSE( mesh_.is_collapse_ok(mesh_.halfedge_handle(1)) ) << "Collapse should be not ok for halfedge 1";
  EXPECT_FALSE( mesh_.is_collapse_ok(mesh_.halfedge_handle(2)) ) << "Collapse should be not ok for halfedge 2";
  EXPECT_FALSE( mesh_.is_collapse_ok(mesh_.halfedge_handle(3)) ) << "Collapse should be not ok for halfedge 3";
  EXPECT_FALSE( mesh_.is_collapse_ok(mesh_.halfedge_handle(4)) ) << "Collapse should be not ok for halfedge 4";
  EXPECT_FALSE( mesh_.is_collapse_ok(mesh_.halfedge_handle(5)) ) << "Collapse should be not ok for halfedge 5";
}

/*
 * Collapsing a tetrahedron
 */
TEST_F(OpenMeshCollapse, CollapseTetrahedron) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[5];

  // Setup a pyramid
  vhandle[0] = mesh_.add_vertex(Mesh::Point(0 , 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1 , 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(0 ,-1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0 , 1, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(-1, 0, 0));

  // Add six faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[0]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_face_status();


  // =============================================
  // Collapse halfedge from 0 to 4
  // =============================================

  Mesh::HalfedgeHandle heh_collapse1 = mesh_.halfedge_handle(0);

  EXPECT_EQ(4, mesh_.to_vertex_handle(heh_collapse1).idx()   ) << "To   vertex of collapse halfedge 1 is wrong";
  EXPECT_EQ(0, mesh_.from_vertex_handle(heh_collapse1).idx() ) << "from vertex of collapse halfedge 1 is wrong";

  EXPECT_TRUE( mesh_.is_collapse_ok(heh_collapse1) ) << "Collapse not ok for collapse first halfedge (0)";
  mesh_.collapse(heh_collapse1);

  Mesh::HalfedgeHandle heh_collapse2 = mesh_.halfedge_handle(2);

  EXPECT_EQ(2, mesh_.to_vertex_handle(heh_collapse2).idx()   ) << "To   vertex of collapse halfedge 2 is wrong";
  EXPECT_EQ(4, mesh_.from_vertex_handle(heh_collapse2).idx() ) << "from vertex of collapse halfedge 2 is wrong";

  EXPECT_TRUE( mesh_.is_collapse_ok(heh_collapse2) ) << "Collapse not ok for collapse second halfedge (2)";
  mesh_.collapse(heh_collapse2);

  Mesh::HalfedgeHandle heh_collapse3 = mesh_.halfedge_handle(6);

  EXPECT_EQ(2, mesh_.to_vertex_handle(heh_collapse3).idx()   ) << "To   vertex of collapse halfedge 3 is wrong";
  EXPECT_EQ(3, mesh_.from_vertex_handle(heh_collapse3).idx() ) << "from vertex of collapse halfedge 3 is wrong";

  EXPECT_FALSE( mesh_.is_collapse_ok(heh_collapse3) ) << "Collapse not ok for collapse third halfedge (6)";


}

/*
 * Test collapsing an halfedge in a triangle mesh
 *
 */
TEST_F(OpenMeshCollapse, LargeCollapseHalfEdge) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[7];

  vhandle[0] = mesh_.add_vertex(Mesh::Point( 0, 1, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point( 1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point( 2, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point( 0,-1, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point( 2,-1, 0));
  vhandle[5] = mesh_.add_vertex(Mesh::Point(-1, 0, 0));
  vhandle[6] = mesh_.add_vertex(Mesh::Point( 3, 0, 0));

  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[1]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

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

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[6]);
  mesh_.add_face(face_vhandles);

  /* Test setup:
      0 ==== 2
     / \    /|\
    /   \  / | \
   5 --- 1   |  6
    \   /  \ | /
     \ /    \|/
      3 ==== 4 */

  // Request the status bits
  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_face_status();

  // =============================================
  // Collapse halfedge from 1 to 4
  // =============================================

  Mesh::HalfedgeHandle heh_collapse;

  // Iterate over all halfedges to find the correct one
  for ( Mesh::HalfedgeIter he_it = mesh_.halfedges_begin() ; he_it != mesh_.halfedges_end() ; ++he_it ) {
    if ( mesh_.from_vertex_handle(*he_it).idx() == 1 && mesh_.to_vertex_handle(*he_it).idx() == 4  )
      heh_collapse = *he_it;
  }

  // Check our halfedge
  EXPECT_EQ(4, mesh_.to_vertex_handle(heh_collapse).idx()   ) << "To   vertex of collapse halfedge is wrong";
  EXPECT_EQ(1, mesh_.from_vertex_handle(heh_collapse).idx() ) << "from vertex of collapse halfedge is wrong";
  EXPECT_TRUE( mesh_.is_collapse_ok(heh_collapse) ) << "Collapse not ok for collapse first halfedge (0)";

  // Remember the end vertices
  Mesh::VertexHandle vh_from = mesh_.from_vertex_handle(heh_collapse);
  Mesh::VertexHandle vh_to   = mesh_.to_vertex_handle(heh_collapse);

  // Collapse it
  mesh_.collapse(heh_collapse);

  EXPECT_TRUE( mesh_.status(vh_from).deleted() ) << "From vertex not deleted";
  EXPECT_FALSE( mesh_.status(vh_to).deleted() )  << "To Vertex deleted";

}


/*
 * Test collapsing an halfedge in a triangle mesh
 *
 */
TEST_F(OpenMeshCollapse, DeletedStatus) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[6];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(-1, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point( 0, 2, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point( 0, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point( 0,-1, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point( 0,-2, 0));
  vhandle[5] = mesh_.add_vertex(Mesh::Point( 1, 0, 0));

  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  /* Test setup:
   *      1
   *     /|\
   *    / | \
   *   / _2_ \
   *  /_/ | \_\
   * 0__  |  __5
   *  \ \_3_/ /
   *   \  |  /
   *    \ | /
   *     \|/
   *      4
   */

  EXPECT_EQ(mesh_.n_faces(), 6u) << "Could not add all faces";

  // Request the status bits
  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_halfedge_status();
  mesh_.request_face_status();

  // =============================================
  // Collapse halfedge from 3 to 2
  // =============================================

  Mesh::HalfedgeHandle heh_collapse = mesh_.find_halfedge(Mesh::VertexHandle(2), Mesh::VertexHandle(3));

  EXPECT_TRUE(heh_collapse.is_valid()) << "Could not find halfedge from vertex 2 to vetex 3";
  EXPECT_TRUE(mesh_.is_collapse_ok(heh_collapse)) << "Collapse is not ok for halfedge from vertex 2 to vertex 3";

  Mesh::HalfedgeHandle top_left     = mesh_.find_halfedge(Mesh::VertexHandle(2), Mesh::VertexHandle(0));
  Mesh::HalfedgeHandle top_right    = mesh_.find_halfedge(Mesh::VertexHandle(5), Mesh::VertexHandle(2));
  Mesh::HalfedgeHandle bottom_left  = mesh_.find_halfedge(Mesh::VertexHandle(0), Mesh::VertexHandle(3));
  Mesh::HalfedgeHandle bottom_right = mesh_.find_halfedge(Mesh::VertexHandle(3), Mesh::VertexHandle(5));

  EXPECT_TRUE(top_left.is_valid())     << "Could not find halfedge from vertex 2 to vetex 0";
  EXPECT_TRUE(top_right.is_valid())    << "Could not find halfedge from vertex 5 to vetex 2";
  EXPECT_TRUE(bottom_left.is_valid())  << "Could not find halfedge from vertex 0 to vetex 3";
  EXPECT_TRUE(bottom_right.is_valid()) << "Could not find halfedge from vertex 3 to vetex 5";

  Mesh::FaceHandle left  = mesh_.face_handle(top_left);
  Mesh::FaceHandle right = mesh_.face_handle(top_right);
  EXPECT_TRUE(left.is_valid())  << "Could not find left face";
  EXPECT_TRUE(right.is_valid()) << "Could not find right";

  mesh_.collapse(heh_collapse);

  EXPECT_TRUE(mesh_.status(Mesh::VertexHandle(2)).deleted()) << "Collapsed vertex is not deleted.";

  EXPECT_TRUE(mesh_.status(left).deleted()) << "Left face is not deleted.";
  EXPECT_TRUE(mesh_.status(right).deleted()) << "Right face is not deleted.";

  EXPECT_TRUE(mesh_.status(mesh_.edge_handle(heh_collapse)).deleted())  << "Collapsed edge is not deleted.";
  EXPECT_TRUE(mesh_.status(mesh_.edge_handle(top_left)).deleted())      << "Top left edge is not deleted.";
  EXPECT_TRUE(mesh_.status(mesh_.edge_handle(top_right)).deleted())     << "Top right edge is not deleted.";
  EXPECT_FALSE(mesh_.status(mesh_.edge_handle(bottom_left)).deleted())  << "Bottom left edge is deleted.";
  EXPECT_FALSE(mesh_.status(mesh_.edge_handle(bottom_right)).deleted()) << "Bottom right edge is deleted.";

  EXPECT_TRUE(mesh_.status(heh_collapse).deleted())                                  << "Collapsed halfedge is not deleted.";
  EXPECT_TRUE(mesh_.status(mesh_.opposite_halfedge_handle(heh_collapse)).deleted())  << "Opposite of collapsed halfedge is not deleted.";

  EXPECT_TRUE(mesh_.status(top_left).deleted())                                      << "Halfedge from vertex 0 to vertex 2 is not deleted";
  EXPECT_TRUE(mesh_.status(mesh_.opposite_halfedge_handle(top_left)).deleted())      << "Halfedge from vertex 2 to vertex 0 is not deleted";
  EXPECT_TRUE(mesh_.status(top_right).deleted())                                     << "Halfedge from vertex 5 to vertex 2 is not deleted";
  EXPECT_TRUE(mesh_.status(mesh_.opposite_halfedge_handle(top_right)).deleted())     << "Halfedge from vertex 2 to vertex 5 is not deleted";
  EXPECT_FALSE(mesh_.status(bottom_left).deleted())                                  << "Halfedge from vertex 0 to vertex 3 is deleted";
  EXPECT_FALSE(mesh_.status(mesh_.opposite_halfedge_handle(bottom_left)).deleted())  << "Halfedge from vertex 3 to vertex 0 is deleted";
  EXPECT_FALSE(mesh_.status(bottom_right).deleted())                                 << "Halfedge from vertex 3 to vertex 5 is deleted";
  EXPECT_FALSE(mesh_.status(mesh_.opposite_halfedge_handle(bottom_right)).deleted()) << "Halfedge from vertex 5 to vertex 3 is deleted";
}

}

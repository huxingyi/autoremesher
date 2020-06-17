#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshIterators : public OpenMeshBase {

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
 * Small VertexIterator Test
 */
TEST_F(OpenMeshIterators, VertexIter) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));

  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  // Test setup:
  //  1 === 2
  //  |   / |    
  //  |  /  |
  //  | /   |
  //  0 === 3

  Mesh::VertexIter v_it  = mesh_.vertices_begin(); 
  Mesh::VertexIter v_end = mesh_.vertices_end(); 

  EXPECT_EQ(0, v_it->idx()) << "Index wrong for vertex iterator vertices_begin()";
  ++v_it;
  EXPECT_EQ(1, v_it->idx()) << "Index wrong in vertex iterator";
  ++v_it;
  EXPECT_EQ(2, v_it->idx()) << "Index wrong in vertex iterator";
  ++v_it;
  EXPECT_EQ(3, v_it->idx()) << "Index wrong in vertex iterator";
  ++v_it;
  EXPECT_EQ(4, v_it->idx()) << "Index wrong in vertex iterator";

  // Check end iterator
  EXPECT_EQ(4, v_end->idx()) << "Index wrong in vertex iterator for vertices_end()";    


  // Second iterator test to start at a specific position

}

/*
 * Small VertexIterator Test to check if we can start iterating at a specific handle
 */
TEST_F(OpenMeshIterators, VertexIterStartPosition) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));

  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  // Test setup:
  //  1 === 2
  //  |   / |
  //  |  /  |
  //  | /   |
  //  0 === 3

  Mesh::VertexIter v_it  = Mesh::VertexIter(mesh_,mesh_.vertex_handle(2));
  Mesh::VertexIter v_end = mesh_.vertices_end();

  EXPECT_EQ(2, v_it->idx()) << "Index wrong in vertex iterator";
  ++v_it;
  EXPECT_EQ(3, v_it->idx()) << "Index wrong in vertex iterator";
  ++v_it;
  EXPECT_EQ(4, v_it->idx()) << "Index wrong in vertex iterator";

  // Check end iterator
  EXPECT_EQ(4, v_end->idx()) << "Index wrong in vertex iterator for vertices_end()";


}


/*
 * Small VertexIterator Test to check if we can continue from the end after adding vertices
 */
TEST_F(OpenMeshIterators, VertexIterContinueAfterEnd) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));

  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  // Test setup:
  //  1 === 2
  //  |   / |
  //  |  /  |
  //  | /   |
  //  0 === 3



  // Add a double vertex property
  OpenMesh::VPropHandleT<int> intHandle;
  EXPECT_FALSE( mesh_.get_property_handle(intHandle,"intProp") );
  mesh_.add_property(intHandle,"intProp");

  // Initialize existing properties:
  int i = 0;
  for (Mesh::VertexIter v_it = mesh_.vertices_begin(); v_it !=  mesh_.vertices_end()  ; ++ v_it) {
    mesh_.property(intHandle,*v_it) = i;
    ++i;
  }

  // Check property
  EXPECT_TRUE(mesh_.get_property_handle(intHandle,"intProp"));

  // Remember old end iterator
  Mesh::VertexIter old_end = mesh_.vertices_end();

  // Set properties to perform test
  for ( unsigned int j = 0 ; j < 4 ; ++j ) {
    OpenMesh::VertexHandle vh = mesh_.add_vertex(Mesh::Point(1, 0, 0));
    mesh_.property(intHandle,vh) = i;
    ++i;
  }

  // Reset to first expected value
  i = 4;

  for ( Mesh::VertexIter test = old_end ; test != mesh_.vertices_end() ; ++test ) {
    EXPECT_EQ( mesh_.property(intHandle,*test) , i);
    ++i;
  }

}

/*
 * Small EdgeIterator Test
 */
TEST_F(OpenMeshIterators, EdgeIter) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));

  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  // Test setup:
  //  1 === 2
  //  |   / |    
  //  |  /  |
  //  | /   |
  //  0 === 3


  Mesh::EdgeIter e_it  = mesh_.edges_begin(); 
  Mesh::EdgeIter e_end = mesh_.edges_end(); 
  
  EXPECT_EQ(0, e_it->idx()) << "Wrong start index in edge iterator";
  EXPECT_EQ(5, e_end->idx()) << "Wrong end index in edge iterator";

  EXPECT_EQ(1, mesh_.to_vertex_handle(mesh_.halfedge_handle(*e_it,0)).idx() )   << "1: Wrong to vertex handle of halfedge 0";
  EXPECT_EQ(2, mesh_.from_vertex_handle(mesh_.halfedge_handle(*e_it,0)).idx() ) << "1: Wrong from vertex handle of halfedge 0";
  EXPECT_EQ(2, mesh_.to_vertex_handle(mesh_.halfedge_handle(*e_it,1)).idx() )   << "1: Wrong to vertex handle of halfedge 1";
  EXPECT_EQ(1, mesh_.from_vertex_handle(mesh_.halfedge_handle(*e_it,1)).idx() ) << "1: Wrong from vertex handle of halfedge 1";

  ++e_it;
  EXPECT_EQ(1, e_it->idx()) << "Wrong index in edge iterator";

  EXPECT_EQ(0, mesh_.to_vertex_handle(mesh_.halfedge_handle(*e_it,0)).idx() )   << "2: Wrong to vertex handle of halfedge 0";
  EXPECT_EQ(1, mesh_.from_vertex_handle(mesh_.halfedge_handle(*e_it,0)).idx() ) << "2: Wrong from vertex handle of halfedge 0";
  EXPECT_EQ(1, mesh_.to_vertex_handle(mesh_.halfedge_handle(*e_it,1)).idx() )   << "2: Wrong to vertex handle of halfedge 1";
  EXPECT_EQ(0, mesh_.from_vertex_handle(mesh_.halfedge_handle(*e_it,1)).idx() ) << "2: Wrong from vertex handle of halfedge 1";


  ++e_it;
  EXPECT_EQ(2, e_it->idx()) << "Wrong index in edge iterator";

  EXPECT_EQ(2, mesh_.to_vertex_handle(mesh_.halfedge_handle(*e_it,0)).idx() )   << "3: Wrong to vertex handle of halfedge 0";
  EXPECT_EQ(0, mesh_.from_vertex_handle(mesh_.halfedge_handle(*e_it,0)).idx() ) << "3: Wrong from vertex handle of halfedge 0";
  EXPECT_EQ(0, mesh_.to_vertex_handle(mesh_.halfedge_handle(*e_it,1)).idx() )   << "3: Wrong to vertex handle of halfedge 1";
  EXPECT_EQ(2, mesh_.from_vertex_handle(mesh_.halfedge_handle(*e_it,1)).idx() ) << "3: Wrong from vertex handle of halfedge 1";


  ++e_it;
  EXPECT_EQ(3, e_it->idx()) << "Wrong index in edge iterator";

  EXPECT_EQ(3, mesh_.to_vertex_handle(mesh_.halfedge_handle(*e_it,0)).idx() )   << "4: Wrong to vertex handle of halfedge 0";
  EXPECT_EQ(0, mesh_.from_vertex_handle(mesh_.halfedge_handle(*e_it,0)).idx() ) << "4: Wrong from vertex handle of halfedge 0";
  EXPECT_EQ(0, mesh_.to_vertex_handle(mesh_.halfedge_handle(*e_it,1)).idx() )   << "4: Wrong to vertex handle of halfedge 1";
  EXPECT_EQ(3, mesh_.from_vertex_handle(mesh_.halfedge_handle(*e_it,1)).idx() ) << "4: Wrong from vertex handle of halfedge 1";


  ++e_it;
  EXPECT_EQ(4, e_it->idx()) << "Wrong index in edge iterator";

  EXPECT_EQ(2, mesh_.to_vertex_handle(mesh_.halfedge_handle(*e_it,0)).idx() )   << "5: Wrong to vertex handle of halfedge 0";
  EXPECT_EQ(3, mesh_.from_vertex_handle(mesh_.halfedge_handle(*e_it,0)).idx() ) << "5: Wrong from vertex handle of halfedge 0";
  EXPECT_EQ(3, mesh_.to_vertex_handle(mesh_.halfedge_handle(*e_it,1)).idx() )   << "5: Wrong to vertex handle of halfedge 1";
  EXPECT_EQ(2, mesh_.from_vertex_handle(mesh_.halfedge_handle(*e_it,1)).idx() ) << "5: Wrong from vertex handle of halfedge 1";


}

/* Adds a cube to a trimesh and runs over all halfedges
 */
TEST_F(OpenMeshIterators, HalfedgeIterSkipping) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[8];
  vhandle[0] = mesh_.add_vertex(Mesh::Point(-1, -1,  1));
  vhandle[1] = mesh_.add_vertex(Mesh::Point( 1, -1,  1));
  vhandle[2] = mesh_.add_vertex(Mesh::Point( 1,  1,  1));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(-1,  1,  1));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(-1, -1, -1));
  vhandle[5] = mesh_.add_vertex(Mesh::Point( 1, -1, -1));
  vhandle[6] = mesh_.add_vertex(Mesh::Point( 1,  1, -1));
  vhandle[7] = mesh_.add_vertex(Mesh::Point(-1,  1, -1));

  // Add six faces to form a cube
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  //=======================

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[5]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  //=======================

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[5]);
  mesh_.add_face(face_vhandles);

  //=======================

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[5]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[6]);
  mesh_.add_face(face_vhandles);


  //=======================

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[6]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[7]);
  mesh_.add_face(face_vhandles);

  //=======================

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[7]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);


  // Test setup:
  //
  //
  //    3 ======== 2
  //   /          /|
  //  /          / |      z
  // 0 ======== 1  |      |
  // |          |  |      |   y
  // |  7       |  6      |  /
  // |          | /       | /
  // |          |/        |/
  // 4 ======== 5         -------> x
  //

  // Check setup
  EXPECT_EQ(18u, mesh_.n_edges() )     << "Wrong number of Edges";
  EXPECT_EQ(36u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices";
  EXPECT_EQ(12u, mesh_.n_faces() )     << "Wrong number of faces";

  mesh_.request_face_status();
  mesh_.request_vertex_status();
  mesh_.request_halfedge_status();

  // Get second edge
  Mesh::EdgeHandle eh = mesh_.edge_handle(2);

  // Delete one edge
  mesh_.delete_edge(eh);

  // Check setup ( No garbage collection, so nothing should change!)
  EXPECT_EQ(18u, mesh_.n_edges() )     << "Wrong number of Edges after delete";
  EXPECT_EQ(36u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges after delete";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices after delete";
  EXPECT_EQ(12u, mesh_.n_faces() )     << "Wrong number of faces after delete";

  Mesh::HalfedgeIter he_it  = mesh_.halfedges_sbegin();
  Mesh::HalfedgeIter he_end = mesh_.halfedges_end();

  EXPECT_EQ(0,  he_it->idx()) << "Wrong start index in halfedge iterator";
  EXPECT_EQ(36, he_end->idx()) << "Wrong end index in halfedge iterator";


  // =====================================================
  // Check skipping iterator
  // =====================================================
  bool ok_4 = true;
  bool ok_5 = true;

  unsigned int count = 0;

  while (he_it != he_end) {
    if ( he_it->idx() ==  4 )
      ok_4 = false;

    if ( he_it->idx() ==  5 )
      ok_5 = false;

    ++he_it;
    ++count;
  }

  EXPECT_EQ(34u,count) << "Skipping iterator count error";
  EXPECT_TRUE( ok_4 )  << "Skipping iterator hit deleted halfedge 4";
  EXPECT_TRUE( ok_5 )  << "Skipping iterator hit deleted halfedge 5";

  // =====================================================
  // Check non skipping iterator
  // =====================================================
  he_it  = mesh_.halfedges_begin();

  count = 0;
  ok_4 = false;
  ok_5 = false;

  while (he_it != he_end) {
    if ( he_it->idx() ==  4 )
      ok_4 = true;

    if ( he_it->idx() ==  5 )
      ok_5 = true;


    ++he_it;
    ++count;
  }

  EXPECT_EQ(36u,count) << "Non-skipping iterator count error";
  EXPECT_TRUE( ok_4 )  << "Non-skipping iterator missed deleted halfedge 4";
  EXPECT_TRUE( ok_5 )  << "Non-skipping iterator missed  deleted halfedge 5";
}

/* Adds a cube to a trimesh and runs over all halfedges
 */
TEST_F(OpenMeshIterators, HalfedgeIterSkippingLowLevel) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[8];
  vhandle[0] = mesh_.add_vertex(Mesh::Point(-1, -1,  1));
  vhandle[1] = mesh_.add_vertex(Mesh::Point( 1, -1,  1));
  vhandle[2] = mesh_.add_vertex(Mesh::Point( 1,  1,  1));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(-1,  1,  1));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(-1, -1, -1));
  vhandle[5] = mesh_.add_vertex(Mesh::Point( 1, -1, -1));
  vhandle[6] = mesh_.add_vertex(Mesh::Point( 1,  1, -1));
  vhandle[7] = mesh_.add_vertex(Mesh::Point(-1,  1, -1));

  // Add six faces to form a cube
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  //=======================

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[5]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  //=======================

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[5]);
  mesh_.add_face(face_vhandles);

  //=======================

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[5]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[6]);
  mesh_.add_face(face_vhandles);


  //=======================

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[6]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[7]);
  mesh_.add_face(face_vhandles);

  //=======================

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[7]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);


  // Test setup:
  //
  //
  //    3 ======== 2
  //   /          /|
  //  /          / |      z
  // 0 ======== 1  |      |
  // |          |  |      |   y
  // |  7       |  6      |  /
  // |          | /       | /
  // |          |/        |/
  // 4 ======== 5         -------> x
  //

  // Check setup
  EXPECT_EQ(18u, mesh_.n_edges() )     << "Wrong number of Edges";
  EXPECT_EQ(36u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices";
  EXPECT_EQ(12u, mesh_.n_faces() )     << "Wrong number of faces";

  mesh_.request_face_status();
  mesh_.request_vertex_status();
  mesh_.request_halfedge_status();

  // Get second edge
  Mesh::EdgeHandle eh = mesh_.edge_handle(2);

  // Check setup ( No garbage collection, so nothing should change!)
  EXPECT_EQ(18u, mesh_.n_edges() )     << "Wrong number of Edges after delete";
  EXPECT_EQ(36u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges after delete";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices after delete";
  EXPECT_EQ(12u, mesh_.n_faces() )     << "Wrong number of faces after delete";

  Mesh::HalfedgeIter he_it  = mesh_.halfedges_sbegin();
  Mesh::HalfedgeIter he_end = mesh_.halfedges_end();

  EXPECT_EQ(0,  he_it->idx()) << "Wrong start index in halfedge iterator";
  EXPECT_EQ(36, he_end->idx()) << "Wrong end index in halfedge iterator";

  // =====================================================
  // Try to add low level edge with invalid incidents and
  // check skipping iterator
  // =====================================================

  // Add a low level edge without handles
  Mesh::EdgeHandle eh_test = mesh_.edge_handle(mesh_.new_edge(Mesh::VertexHandle(),Mesh::VertexHandle()));

  he_it  = mesh_.halfedges_sbegin();
  he_end = mesh_.halfedges_end();

  unsigned int count = 0;
  bool found_36 = false;
  bool found_37 = false;

  while (he_it != he_end) {
    if ( he_it->idx() ==  36 )
      found_36 = true;

    if ( he_it->idx() ==  37 )
      found_37 = true;

    ++he_it;
    ++count;
  }

  EXPECT_EQ(38u,count) << "Skipping iterator count error";
  EXPECT_TRUE( found_36 )  << "Skipping iterator missed halfedge 36";
  EXPECT_TRUE( found_37 )  << "Skipping iterator missed halfedge 37";

  // =====================================================
  // Try to delete one edge with valid incidents and
  // check skipping iterator
  // =====================================================

  // delete one edge and recheck (Halfedges 4 and 5)
  mesh_.delete_edge(eh);

  he_it  = mesh_.halfedges_sbegin();
  he_end = mesh_.halfedges_end();

  count = 0;
  bool found_4  = false;
  bool found_5  = false;
  found_36 = false;
  found_37 = false;

  while (he_it != he_end) {

    if ( he_it->idx() ==  4 )
      found_4 = true;

    if ( he_it->idx() ==  5 )
      found_5 = true;

    if ( he_it->idx() ==  36 )
      found_36 = true;

    if ( he_it->idx() ==  37 )
      found_37 = true;

    ++he_it;
    ++count;
  }

  EXPECT_EQ(36u,count) << "Deleted valid: Skipping iterator count error";
  EXPECT_FALSE( found_4 )  << "Deleted valid: Skipping iterator hit deleted halfedge 4";
  EXPECT_FALSE( found_5 )  << "Deleted valid: Skipping iterator hit  deleted halfedge 5";
  EXPECT_TRUE( found_36 )  << "Deleted valid: Skipping iterator missed halfedge 36";
  EXPECT_TRUE( found_37 )  << "Deleted valid: Skipping iterator missed halfedge 37";

  // =====================================================
  // Try to delete one edge with invalid incidents and
  // check skipping iterator
  // =====================================================

  // delete one edge and recheck (Halfedges 4 and 5)
  mesh_.delete_edge(eh_test);

  he_it  = mesh_.halfedges_sbegin();
  he_end = mesh_.halfedges_end();

  count = 0;
  found_4  = false;
  found_5  = false;
  found_36 = false;
  found_37 = false;

  while (he_it != he_end) {

    if ( he_it->idx() ==  4 )
      found_4 = true;

    if ( he_it->idx() ==  5 )
      found_5 = true;

    if ( he_it->idx() ==  36 )
      found_36 = true;

    if ( he_it->idx() ==  37 )
      found_37 = true;

    ++he_it;
    ++count;
  }

  EXPECT_EQ(34u,count) << "Deleted invalid: Skipping iterator count error";
  EXPECT_FALSE( found_4 )  << "Deleted invalid: Skipping iterator hit deleted halfedge 4";
  EXPECT_FALSE( found_5 )  << "Deleted invalid: Skipping iterator hit deleted halfedge 5";
  EXPECT_FALSE( found_36 ) << "Deleted invalid: Skipping iterator hit deleted halfedge 36";
  EXPECT_FALSE( found_37 ) << "Deleted invalid: Skipping iterator hit deleted halfedge 37";

}

/*
 * Test with a mesh with one deleted face
 */
TEST_F(OpenMeshIterators, FaceIterEmptyMeshOneDeletedFace) {

  mesh_.clear();

  // request delete_face capability
  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_face_status();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));

  // Add one faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  Mesh::FaceHandle fh = mesh_.add_face(face_vhandles);

  // Delete face but keep vertices
  bool const is_delete_isolated_vertex = false;
  mesh_.delete_face(fh, is_delete_isolated_vertex);

  // Test setup (Face deleted but vertices kept.
  //  1 === 2
  //  |   /
  //  |  /
  //  | /
  //  0

  Mesh::FaceIter f_it  = mesh_.faces_begin();
  Mesh::FaceIter f_end = mesh_.faces_end();

  EXPECT_EQ(0, f_it->idx()) << "Wrong start index in FaceIterator";

  EXPECT_EQ(1, f_end->idx()) << "Wrong end index in FaceIterator";

  ++f_it;
  EXPECT_EQ(1, f_it->idx()) << "Wrong end index in FaceIterator after one step";
  EXPECT_TRUE(f_it == f_end ) << "Iterator not at end for FaceIterator after one step";

  Mesh::ConstFaceIter cf_it  = mesh_.faces_begin();
  Mesh::ConstFaceIter cf_end = mesh_.faces_end();

  EXPECT_EQ(0, cf_it->idx()) << "Wrong start index in ConstFaceIterator";

  EXPECT_EQ(1, cf_end->idx()) << "Wrong end index in ConstFaceIterator";

  ++cf_it;
  EXPECT_EQ(1, cf_it->idx()) << "Wrong end index in ConstFaceIterator after one step";
  EXPECT_TRUE(cf_it == cf_end ) << "Iterator not at end for ConstFaceIterator after one step";


  // Same with skipping iterators:
  f_it  = mesh_.faces_sbegin();
  f_end = mesh_.faces_end();

  EXPECT_EQ(1, f_it->idx()) << "Wrong start index in FaceIterator with skipping";

  EXPECT_EQ(1, f_end->idx()) << "Wrong end index in FaceIterator with skipping";

  EXPECT_TRUE(f_it == f_end ) << "Iterator not at end for FaceIterator with skipping";

  // Same with skipping iterators:
  cf_it  = mesh_.faces_sbegin();
  cf_end = mesh_.faces_end();

  EXPECT_EQ(1, cf_it->idx()) << "Wrong start index in ConstFaceIterator with skipping";

  EXPECT_EQ(1, cf_end->idx()) << "Wrong end index in ConstFaceIterator with skipping";

  EXPECT_TRUE(cf_it == cf_end ) << "Iterator not at end for ConstFaceIterator with skipping";


  mesh_.release_vertex_status();
  mesh_.release_edge_status();
  mesh_.release_face_status();

}



/*
 * Test range iterators
 */
TEST_F(OpenMeshIterators, RangeIterators) {

  mesh_.clear();

  // request delete_face capability
  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_halfedge_status();
  mesh_.request_face_status();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));

  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  Mesh::FaceHandle fh = mesh_.add_face(face_vhandles);

  // Delete one face
  mesh_.delete_face(fh);

  // Test setup (right face deleted)
  //  1 --- 2
  //  |   / |
  //  |  /  |
  //  | /   |
  //  0 --- 3


  // ====== Faces ======

  int count_faces_iter = 0;
  for (auto f_it = mesh_.faces_begin(); f_it != mesh_.faces_end(); ++f_it)
    ++count_faces_iter;
  EXPECT_EQ(2, count_faces_iter) << "Wrong number of visited faces.";

  int count_faces_skipping_iter = 0;
  for (auto f_it = mesh_.faces_sbegin(); f_it != mesh_.faces_end(); ++f_it)
    ++count_faces_skipping_iter;
  EXPECT_EQ(1, count_faces_skipping_iter) << "Wrong number of visited faces.";

  int count_faces_range = 0;
  for (auto fh : mesh_.faces())
    if (fh.is_valid()) // not actually necessary but fixes unused variable warning
      ++count_faces_range;
  EXPECT_EQ(1, count_faces_range) << "Wrong number of visited faces.";

  int count_faces_range_all = 0;
  for (auto fh : mesh_.all_faces())
    if (fh.is_valid()) // not actually necessary but fixes unused variable warning
      ++count_faces_range_all;
  EXPECT_EQ(2, count_faces_range_all) << "Wrong number of visited faces.";


  // ====== Edges ======

  int count_edges_iter = 0;
  for (auto e_it = mesh_.edges_begin(); e_it != mesh_.edges_end(); ++e_it)
    ++count_edges_iter;
  EXPECT_EQ(5, count_edges_iter) << "Wrong number of visited edges.";

  int count_edges_skipping_iter = 0;
  for (auto e_it = mesh_.edges_sbegin(); e_it != mesh_.edges_end(); ++e_it)
    ++count_edges_skipping_iter;
  EXPECT_EQ(3, count_edges_skipping_iter) << "Wrong number of visited edges.";

  int count_edges_range = 0;
  for (auto eh : mesh_.edges())
    if (eh.is_valid()) // not actually necessary but fixes unused variable warning
      ++count_edges_range;
  EXPECT_EQ(3, count_edges_range) << "Wrong number of visited edges.";

  int count_edges_range_all = 0;
  for (auto eh : mesh_.all_edges())
    if (eh.is_valid()) // not actually necessary but fixes unused variable warning
      ++count_edges_range_all;
  EXPECT_EQ(5, count_edges_range_all) << "Wrong number of visited edges.";


  // ====== Halfedges ======

  int count_halfedges_iter = 0;
  for (auto h_it = mesh_.halfedges_begin(); h_it != mesh_.halfedges_end(); ++h_it)
    ++count_halfedges_iter;
  EXPECT_EQ(10, count_halfedges_iter) << "Wrong number of visited halfedges.";

  int count_halfedges_skipping_iter = 0;
  for (auto h_it = mesh_.halfedges_sbegin(); h_it != mesh_.halfedges_end(); ++h_it)
    ++count_halfedges_skipping_iter;
  EXPECT_EQ(6, count_halfedges_skipping_iter) << "Wrong number of visited halfedges.";

  int count_halfedges_range = 0;
  for (auto heh : mesh_.halfedges())
    if (heh.is_valid()) // not actually necessary but fixes unused variable warning
      ++count_halfedges_range;
  EXPECT_EQ(6, count_halfedges_range) << "Wrong number of visited halfedges.";

  int count_halfedges_range_all = 0;
  for (auto heh : mesh_.all_halfedges())
    if (heh.is_valid()) // not actually necessary but fixes unused variable warning
      ++count_halfedges_range_all;
  EXPECT_EQ(10, count_halfedges_range_all) << "Wrong number of visited halfedges.";


  // ====== Vertices ======

  int count_vertices_iter = 0;
  for (auto v_it = mesh_.vertices_begin(); v_it != mesh_.vertices_end(); ++v_it)
    ++count_vertices_iter;
  EXPECT_EQ(4, count_vertices_iter) << "Wrong number of visited vertices.";

  int count_vertices_skipping_iter = 0;
  for (auto v_it = mesh_.vertices_sbegin(); v_it != mesh_.vertices_end(); ++v_it)
    ++count_vertices_skipping_iter;
  EXPECT_EQ(3, count_vertices_skipping_iter) << "Wrong number of visited vertices.";

  int count_vertices_range = 0;
  for (auto vh : mesh_.vertices())
    if (vh.is_valid()) // not actually necessary but fixes unused variable warning
      ++count_vertices_range;
  EXPECT_EQ(3, count_vertices_range) << "Wrong number of visited vertices.";

  int count_vertices_range_all = 0;
  for (auto vh : mesh_.all_vertices())
    if (vh.is_valid()) // not actually necessary but fixes unused variable warning
      ++count_vertices_range_all;
  EXPECT_EQ(4, count_vertices_range_all) << "Wrong number of visited vertices.";


  mesh_.release_vertex_status();
  mesh_.release_edge_status();
  mesh_.release_halfedge_status();
  mesh_.release_face_status();

}



}

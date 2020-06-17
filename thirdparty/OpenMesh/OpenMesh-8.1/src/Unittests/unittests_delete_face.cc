
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <iostream>

namespace {

class OpenMeshDeleteFaceTriangleMesh : public OpenMeshBase {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
            
            // Do some initial stuff with the member data here...
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }

    // Member already defined in OpenMeshBase
    //Mesh mesh_;  
};

class OpenMeshDeleteFacePolyMesh : public OpenMeshBasePoly {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
            
            // Do some initial stuff with the member data here...
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

/* Adds a cube to a trimesh and then deletes half of the faces
 * It does not request edge status!
 */
TEST_F(OpenMeshDeleteFaceTriangleMesh, DeleteHalfTriangleMeshCubeNoEdgeStatus) {

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

  // =====================================================
  // Now we delete half of the mesh
  // =====================================================
  mesh_.request_face_status();
  mesh_.request_vertex_status();
  mesh_.request_halfedge_status();
  const size_t n_face_to_delete = mesh_.n_faces()/2;

  // Check the variable
  EXPECT_EQ(6u, n_face_to_delete )    << "Wrong number of faces to delete";

  for(size_t i = 0; i < n_face_to_delete; i++)
	  mesh_.delete_face(mesh_.face_handle(int(i)));

  // =====================================================
  // Check config afterwards
  // =====================================================

  EXPECT_EQ(18u, mesh_.n_edges() )     << "Wrong number of Edges after marking as deleted";
  EXPECT_EQ(36u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges after marking as deleted";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices after marking as deleted";
  EXPECT_EQ(12u, mesh_.n_faces() )     << "Wrong number of faces after marking as deleted";


  // =====================================================
  // Cleanup and recheck
  // =====================================================
  mesh_.garbage_collection();

  EXPECT_EQ(18u, mesh_.n_edges() )     << "Wrong number of Edges after garbage collection";
  EXPECT_EQ(36u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges after garbage collection";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices after garbage collection";
  EXPECT_EQ(6u, mesh_.n_faces() )      << "Wrong number of faces after garbage collection";
}


/* Adds a cube to a trimesh and then deletes half of the faces
 */
TEST_F(OpenMeshDeleteFaceTriangleMesh, DeleteHalfTriangleMeshCubeWithEdgeStatus) {

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

  // =====================================================
  // Now we delete half of the mesh
  // =====================================================
  mesh_.request_face_status();
  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_halfedge_status();
  const size_t n_face_to_delete = mesh_.n_faces()/2;

  // Check the variable
  EXPECT_EQ(6u, n_face_to_delete )    << "Wrong number of faces to delete";

  for(size_t i = 0; i < n_face_to_delete; i++)
	  mesh_.delete_face(mesh_.face_handle(int(i)));

  // =====================================================
  // Check config afterwards
  // =====================================================

  EXPECT_EQ(18u, mesh_.n_edges() )     << "Wrong number of Edges after marking as deleted";
  EXPECT_EQ(36u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges after marking as deleted";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices after marking as deleted";
  EXPECT_EQ(12u, mesh_.n_faces() )     << "Wrong number of faces after marking as deleted";


  // =====================================================
  // Cleanup and recheck
  // =====================================================
  mesh_.garbage_collection();

  EXPECT_EQ(13u, mesh_.n_edges() )     << "Wrong number of Edges after garbage collection";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices after garbage collection";
  EXPECT_EQ(6u, mesh_.n_faces() )      << "Wrong number of faces after garbage collection";
}



/* Adds a cube to a polymesh
 * And delete half of it.
 * Does not request edge status
 */
TEST_F(OpenMeshDeleteFacePolyMesh, DeleteteHalfPolyMeshCubeWithoutEdgeStatus) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[8];
  vhandle[0] = mesh_.add_vertex(PolyMesh::Point(-1, -1,  1));
  vhandle[1] = mesh_.add_vertex(PolyMesh::Point( 1, -1,  1));
  vhandle[2] = mesh_.add_vertex(PolyMesh::Point( 1,  1,  1));
  vhandle[3] = mesh_.add_vertex(PolyMesh::Point(-1,  1,  1));
  vhandle[4] = mesh_.add_vertex(PolyMesh::Point(-1, -1, -1));
  vhandle[5] = mesh_.add_vertex(PolyMesh::Point( 1, -1, -1));
  vhandle[6] = mesh_.add_vertex(PolyMesh::Point( 1,  1, -1));
  vhandle[7] = mesh_.add_vertex(PolyMesh::Point(-1,  1, -1));

  // Add six faces to form a cube
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[5]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[6]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[7]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
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
  EXPECT_EQ(12u, mesh_.n_edges() )     << "Wrong number of Edges";
  EXPECT_EQ(24u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices";
  EXPECT_EQ(6u, mesh_.n_faces() )      << "Wrong number of faces";

  // =====================================================
  // Now we delete half of the mesh
  // =====================================================
  mesh_.request_face_status();
  mesh_.request_vertex_status();
  mesh_.request_halfedge_status();
  const size_t n_face_to_delete = mesh_.n_faces()/2;

  // Check the variable
  EXPECT_EQ(3u, n_face_to_delete )    << "Wrong number of faces to delete";

  for(size_t i = 0; i < n_face_to_delete; i++)
	  mesh_.delete_face(mesh_.face_handle(int(i)));

  // =====================================================
  // Check config afterwards
  // =====================================================

  EXPECT_EQ(12u, mesh_.n_edges() )     << "Wrong number of Edges after marking as deleted";
  EXPECT_EQ(24u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges after marking as deleted";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices after marking as deleted";
  EXPECT_EQ(6u, mesh_.n_faces() )      << "Wrong number of faces after marking as deleted";

  // =====================================================
  // Cleanup and recheck
  // =====================================================
  mesh_.garbage_collection();

  EXPECT_EQ(12u, mesh_.n_edges() )     << "Wrong number of Edges after garbage collection";
  EXPECT_EQ(24u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges after garbage collection";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices after garbage collection";
  EXPECT_EQ(3u, mesh_.n_faces() )      << "Wrong number of faces after garbage collection";

}

/* Adds a cube to a polymesh
 * And delete half of it.
 */
TEST_F(OpenMeshDeleteFacePolyMesh, DeleteteHalfPolyMeshCubeWithEdgeStatus) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[8];
  vhandle[0] = mesh_.add_vertex(PolyMesh::Point(-1, -1,  1));
  vhandle[1] = mesh_.add_vertex(PolyMesh::Point( 1, -1,  1));
  vhandle[2] = mesh_.add_vertex(PolyMesh::Point( 1,  1,  1));
  vhandle[3] = mesh_.add_vertex(PolyMesh::Point(-1,  1,  1));
  vhandle[4] = mesh_.add_vertex(PolyMesh::Point(-1, -1, -1));
  vhandle[5] = mesh_.add_vertex(PolyMesh::Point( 1, -1, -1));
  vhandle[6] = mesh_.add_vertex(PolyMesh::Point( 1,  1, -1));
  vhandle[7] = mesh_.add_vertex(PolyMesh::Point(-1,  1, -1));

  // Add six faces to form a cube
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);
 
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[5]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[6]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[7]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
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
  EXPECT_EQ(12u, mesh_.n_edges() )     << "Wrong number of Edges";
  EXPECT_EQ(24u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices";
  EXPECT_EQ(6u, mesh_.n_faces() )      << "Wrong number of faces";

  // =====================================================
  // Now we delete half of the mesh
  // =====================================================
  mesh_.request_face_status();
  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_halfedge_status();
  const size_t n_face_to_delete = mesh_.n_faces()/2;

  // Check the variable
  EXPECT_EQ(3u, n_face_to_delete )    << "Wrong number of faces to delete";

  for(size_t i = 0; i < n_face_to_delete; i++)
	  mesh_.delete_face(mesh_.face_handle(int(i)));

  // =====================================================
  // Check config afterwards
  // =====================================================

  EXPECT_EQ(12u, mesh_.n_edges() )     << "Wrong number of Edges after marking as deleted";
  EXPECT_EQ(24u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges after marking as deleted";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices after marking as deleted";
  EXPECT_EQ(6u, mesh_.n_faces() )      << "Wrong number of faces after marking as deleted";

  // =====================================================
  // Cleanup and recheck
  // =====================================================
  mesh_.garbage_collection();

  EXPECT_EQ(10u, mesh_.n_edges() )     << "Wrong number of Edges after garbage collection";
  EXPECT_EQ(20u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges after garbage collection";
  EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices after garbage collection";
  EXPECT_EQ(3u, mesh_.n_faces() )      << "Wrong number of faces after garbage collection";

}

}

#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshTriMeshGarbageCollection : public OpenMeshBase {

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

/* Adds a cube to a trimesh and deletes vertex 0 and calls garbage collection afterwards
 */
TEST_F(OpenMeshTriMeshGarbageCollection, StandardGarbageCollection) {

  mesh_.clear();

  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_halfedge_status();
  mesh_.request_face_status();

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
  EXPECT_EQ(8u, mesh_.n_vertices() ) << "Wrong initial number of vertices";
  EXPECT_EQ(12u, mesh_.n_faces() )   << "Wrong initial number of faces";

  mesh_.delete_vertex(vhandle[0]);

  // Check setup
  EXPECT_EQ(8u, mesh_.n_vertices() ) << "Wrong number of vertices after deletion";
  EXPECT_EQ(12u, mesh_.n_faces() )   << "Wrong number of faces after deletion";

  mesh_.garbage_collection();

  // Check setup
  EXPECT_EQ(7u, mesh_.n_vertices() ) << "Wrong number of vertices after garbage collection";
  EXPECT_EQ(8u, mesh_.n_faces() )   << "Wrong number of faces after garbage collection";

}

/* Adds a cube to a trimesh and deletes vertex 0 and calls garbage collection afterwards.
 * But this time, we track the vertex handles , halfedge handles, and face handles
 */
TEST_F(OpenMeshTriMeshGarbageCollection, TrackedGarbageCollection) {

  mesh_.clear();

  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_halfedge_status();
  mesh_.request_face_status();

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

  // 0
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  // 1
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  //=======================

  // 2
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[5]);
  mesh_.add_face(face_vhandles);

  // 3
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  //=======================

  // 4
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  // 5
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[5]);
  mesh_.add_face(face_vhandles);

  //=======================

  // 6
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[5]);
  mesh_.add_face(face_vhandles);

  // 7
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[6]);
  mesh_.add_face(face_vhandles);


  //=======================

  // 8
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[6]);
  mesh_.add_face(face_vhandles);

  // 9
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[7]);
  mesh_.add_face(face_vhandles);

  //=======================

  // 10
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[7]);
  mesh_.add_face(face_vhandles);

  // 11
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
  EXPECT_EQ(8u, mesh_.n_vertices() ) << "Wrong initial number of vertices";
  EXPECT_EQ(12u, mesh_.n_faces() )   << "Wrong initial number of faces";

  //==================================================
  // Create vectors containing the current handles
  //==================================================
  std::vector<Mesh::VertexHandle>   vertexHandles;
  for ( Mesh::VertexIter v_it = mesh_.vertices_begin(); v_it != mesh_.vertices_end(); ++v_it)
    vertexHandles.push_back(*v_it);

  std::vector<Mesh::HalfedgeHandle> halfedgeHandles;
  for ( Mesh::HalfedgeIter he_it = mesh_.halfedges_begin(); he_it != mesh_.halfedges_end(); ++he_it)
    halfedgeHandles.push_back(*he_it);

  std::vector<Mesh::FaceHandle>     faceHandles;
  for ( Mesh::FaceIter f_it = mesh_.faces_begin(); f_it != mesh_.faces_end(); ++f_it)
    faceHandles.push_back(*f_it);

  //==================================================
  // Create vectors containing pointers current handles
  //==================================================
  std::vector<Mesh::VertexHandle*>   vertexHandlesP;
  for ( unsigned int i = 0 ; i < vertexHandles.size() ; ++i)
    vertexHandlesP.push_back(&(vertexHandles[i]));

  std::vector<Mesh::HalfedgeHandle*> halfedgeHandlesP;
  for ( unsigned int i = 0 ; i < halfedgeHandles.size() ; ++i) {
    halfedgeHandlesP.push_back(&(halfedgeHandles[i]));
  }

  std::vector<Mesh::FaceHandle*>     faceHandlesP;
  for ( unsigned int i = 0 ; i < faceHandles.size() ; ++i)
    faceHandlesP.push_back(&(faceHandles[i]));

//  // REMOVE
//
//  OpenMesh::HPropHandleT<int> handle;
//   mesh_.add_property(handle,"tmp");
//
//
//  int count = 0;
//
//  for ( Mesh::HalfedgeIter he_it = mesh_.halfedges_begin(); he_it !=  mesh_.halfedges_end() ; ++he_it) {
//    mesh_.property(handle,he_it) =count;
//    ++count;
//  }

//  std::cerr << "Vertex   : ";
//  for ( unsigned int i = 0 ; i < vertexHandles.size() ; ++i)
//    std::cerr <<  vertexHandles[i].idx() << " ";
//  std::cerr << std::endl;
//
//  std::cerr << "Halfedge : ";
//  for ( unsigned int i = 0 ; i < halfedgeHandles.size() ; ++i)
//    std::cerr <<  halfedgeHandles[i].idx() << " ";
//  std::cerr << std::endl;
//
//  std::cerr << "Halfedge property : ";
//  for ( Mesh::HalfedgeIter he_it = mesh_.halfedges_begin(); he_it !=  mesh_.halfedges_end() ; ++he_it) {
//    std::cerr <<  mesh_.property(handle,he_it) << " ";
//  }
//  std::cerr << std::endl;
//
//  std::cerr << "Face     : ";
//  for ( unsigned int i = 0 ; i < faceHandles.size() ; ++i)
//    std::cerr <<  faceHandles[i].idx() << " ";
//  std::cerr << std::endl;
  // REMOVE END


  // Deleting vertex 0
  // -> deletes vertex 0
  // -> deletes faces 0,4,10,11
  mesh_.delete_vertex(vhandle[0]);
  //mesh_.delete_vertex(vhandle[7]);

  // Check setup
  EXPECT_EQ(8u, mesh_.n_vertices() ) << "Wrong number of vertices after deletion";
  EXPECT_EQ(12u, mesh_.n_faces() )   << "Wrong number of faces after deletion";

  mesh_.garbage_collection(vertexHandlesP,halfedgeHandlesP,faceHandlesP,true,true,true);

  // Check setup
  EXPECT_EQ(7u, mesh_.n_vertices() ) << "Wrong number of vertices after garbage collection";
  EXPECT_EQ(8u, mesh_.n_faces() )   << "Wrong number of faces after garbage collection";

  //================================
  // Check the updated handles
  //================================

//  //  // REMOVE
//  std::cerr << "Vertex   : ";
//  for ( unsigned int i = 0 ; i < vertexHandles.size() ; ++i)
//    std::cerr <<  vertexHandles[i].idx() << " ";
//  std::cerr << std::endl;
//
//  std::cerr << "Halfedge : ";
//  for ( unsigned int i = 0 ; i < halfedgeHandles.size() ; ++i)
//    std::cerr <<  halfedgeHandles[i].idx() << " ";
//  std::cerr << std::endl;
//
//  std::cerr << "Halfedge property : ";
//  for ( Mesh::HalfedgeIter he_it = mesh_.halfedges_begin(); he_it !=  mesh_.halfedges_end() ; ++he_it) {
//    std::cerr <<  mesh_.property(handle,he_it) << " ";
//  }
//  std::cerr << std::endl;
//
//  std::cerr << "Face     : ";
//  for ( unsigned int i = 0 ; i < faceHandles.size() ; ++i)
//    std::cerr <<  faceHandles[i].idx() << " ";
//  std::cerr << std::endl;
  // REMOVE END

  // Check setup of vertices
  EXPECT_EQ(-1, vertexHandles[0].idx() )  << "Wrong vertex handle after update";
  EXPECT_EQ(1 , vertexHandles[1].idx() )  << "Wrong vertex handle after update";
  EXPECT_EQ(2 , vertexHandles[2].idx() )  << "Wrong vertex handle after update";
  EXPECT_EQ(3 , vertexHandles[3].idx() )  << "Wrong vertex handle after update";
  EXPECT_EQ(4 , vertexHandles[4].idx() )  << "Wrong vertex handle after update";
  EXPECT_EQ(5 , vertexHandles[5].idx() )  << "Wrong vertex handle after update";
  EXPECT_EQ(6 , vertexHandles[6].idx() )  << "Wrong vertex handle after update";
  EXPECT_EQ(0 , vertexHandles[7].idx() )  << "Wrong vertex handle after update";

  // Check setup of halfedge handles
  EXPECT_EQ(-1, halfedgeHandles[0 ].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(-1, halfedgeHandles[1 ].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ( 2, halfedgeHandles[2 ].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ( 3, halfedgeHandles[3 ].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(-1, halfedgeHandles[4 ].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(-1, halfedgeHandles[5 ].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ( 6, halfedgeHandles[6 ].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ( 7, halfedgeHandles[7 ].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ( 8, halfedgeHandles[8 ].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ( 9, halfedgeHandles[9 ].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(10, halfedgeHandles[10].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(11, halfedgeHandles[11].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(12, halfedgeHandles[12].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(13, halfedgeHandles[13].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(14, halfedgeHandles[14].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(15, halfedgeHandles[15].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(16, halfedgeHandles[16].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(17, halfedgeHandles[17].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(18, halfedgeHandles[18].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(19, halfedgeHandles[19].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(-1, halfedgeHandles[20].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(-1, halfedgeHandles[21].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(22, halfedgeHandles[22].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(23, halfedgeHandles[23].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(24, halfedgeHandles[24].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(25, halfedgeHandles[25].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(26, halfedgeHandles[26].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(27, halfedgeHandles[27].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(20, halfedgeHandles[28].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(21, halfedgeHandles[29].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ( 4, halfedgeHandles[30].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ( 5, halfedgeHandles[31].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ( 0, halfedgeHandles[32].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ( 1, halfedgeHandles[33].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(-1, halfedgeHandles[34].idx() )  << "Wrong halfedge handle after update";
  EXPECT_EQ(-1, halfedgeHandles[35].idx() )  << "Wrong halfedge handle after update";

  // Check setup of faces
  EXPECT_EQ(-1 , faceHandles[0 ].idx() )  << "Wrong face handle after update";
  EXPECT_EQ(1  , faceHandles[1 ].idx() )  << "Wrong face handle after update";
  EXPECT_EQ(2  , faceHandles[2 ].idx() )  << "Wrong face handle after update";
  EXPECT_EQ(3  , faceHandles[3 ].idx() )  << "Wrong face handle after update";
  EXPECT_EQ(-1 , faceHandles[4 ].idx() )  << "Wrong face handle after update";
  EXPECT_EQ(5  , faceHandles[5 ].idx() )  << "Wrong face handle after update";
  EXPECT_EQ(6  , faceHandles[6 ].idx() )  << "Wrong face handle after update";
  EXPECT_EQ(7  , faceHandles[7 ].idx() )  << "Wrong face handle after update";
  EXPECT_EQ(4  , faceHandles[8 ].idx() )  << "Wrong face handle after update";
  EXPECT_EQ(0  , faceHandles[9 ].idx() )  << "Wrong face handle after update";
  EXPECT_EQ(-1 , faceHandles[10].idx() )  << "Wrong face handle after update";
  EXPECT_EQ(-1 , faceHandles[11].idx() )  << "Wrong face handle after update";

}


}

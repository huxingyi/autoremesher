
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <iostream>

namespace {

class OpenMeshSplitEdgeCopyTriangleMesh : public OpenMeshBase {

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

class OpenMeshSplitEdgeCopyPolyMesh : public OpenMeshBasePoly {

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

/* splits an edge that has a property in a triangle mesh with split_edge_copy
 * the property should be copied to the new edge
 */
TEST_F(OpenMeshSplitEdgeCopyTriangleMesh, SplitEdgeCopyTriangleMesh) {

  mesh_.clear();
  mesh_.request_edge_status();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0.25, 0.25, 0));

  // Add one face
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);

  Mesh::FaceHandle fh = mesh_.add_face(face_vhandles);
  Mesh::EdgeHandle eh = *mesh_.edges_begin();

  // Test setup:
  //  1 === 2
  //  |   /
  //  |  /
  //  | /
  //  0

  // set property
  OpenMesh::EPropHandleT<int> eprop_int;
  mesh_.add_property(eprop_int);
  mesh_.property(eprop_int, eh) = 999;
  //set internal property
  mesh_.status(eh).set_tagged(true);

  // split edge with new vertex
  mesh_.split_copy(eh, vhandle[3]);

  // Check setup
  Mesh::EdgeHandle eh0 = mesh_.edge_handle( mesh_.next_halfedge_handle( mesh_.halfedge_handle(eh, 1) ) );
  EXPECT_EQ(999, mesh_.property(eprop_int, eh0)) << "Different Property value";
  EXPECT_TRUE(mesh_.status(eh0).tagged()) << "Different internal property value";

  EXPECT_EQ(3u, mesh_.valence(fh)) << "Face of TriMesh has valence other than 3";
}

/* splits an edge that has a property in a poly mesh with split_edge_copy
 * the property should be copied to the new faces
 */
TEST_F(OpenMeshSplitEdgeCopyPolyMesh, SplitEdgeCopyPolymesh) {

  mesh_.clear();
  mesh_.request_edge_status();

  // Add some vertices
  Mesh::VertexHandle vhandle[5];

  vhandle[0] = mesh_.add_vertex(PolyMesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(PolyMesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(PolyMesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(PolyMesh::Point(1, 0, 0));
  vhandle[4] = mesh_.add_vertex(PolyMesh::Point(0.5, 0.5, 0));

  // Add face
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);

  mesh_.add_face(face_vhandles);
  PolyMesh::EdgeHandle eh = *mesh_.edges_begin();

  // Test setup:
  //  1 === 2
  //  |     |
  //  |     |
  //  |     |
  //  0 === 3

  // set property
  OpenMesh::EPropHandleT<int> eprop_int;
  mesh_.add_property(eprop_int);
  mesh_.property(eprop_int, eh) = 999;
  //set internal property
  mesh_.status(eh).set_tagged(true);


  // split face with new vertex
  mesh_.split_edge_copy(eh, vhandle[4]);


  // Check setup  
  Mesh::EdgeHandle eh0 = mesh_.edge_handle( mesh_.next_halfedge_handle( mesh_.halfedge_handle(eh, 1) ) );
  EXPECT_EQ(999, mesh_.property(eprop_int, eh0)) << "Different Property value";
  EXPECT_TRUE(mesh_.status(eh0).tagged()) << "Different internal property value";
}


/* splits an edge in a triangle mesh that has a face property with split_edge_copy
 * the property should be copied to the new edge
 */
TEST_F(OpenMeshSplitEdgeCopyTriangleMesh, SplitEdgeCopyFacePropertiesTriangleMesh) {

  mesh_.clear();
  mesh_.request_edge_status();
  mesh_.request_face_status();

  static_assert(std::is_same<decltype (mesh_), Mesh>::value, "Mesh is not a triangle mesh");

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));

  Mesh::VertexHandle inner_vertex    = mesh_.add_vertex(Mesh::Point(0.5, 0.5, 0));
  Mesh::VertexHandle boundary_vertex = mesh_.add_vertex(Mesh::Point(0.0, 0.5, 0));

  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);

  Mesh::FaceHandle fh0 = mesh_.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);

  Mesh::FaceHandle fh1 = mesh_.add_face(face_vhandles);

  Mesh::EdgeHandle inner_edge = Mesh::EdgeHandle(2);

  EXPECT_EQ(mesh_.n_faces(), 2u);

  // Test setup:
  //  1 --- 2
  //  |   / |
  //  |  /  |
  //  | /   |
  //  0 --- 3

  // set property
  OpenMesh::FPropHandleT<int> fprop_int;
  mesh_.add_property(fprop_int);
  mesh_.property(fprop_int, fh0) = 13;
  mesh_.property(fprop_int, fh1) = 17;
  //set internal property
  mesh_.status(fh0).set_tagged(true);

  // 2 to 4 split
  mesh_.split_copy(inner_edge, inner_vertex);

  EXPECT_EQ(mesh_.n_faces(), 4u);

  for (auto fh : mesh_.faces())
  {
    EXPECT_EQ(3u, mesh_.valence(fh));
  }

  // Check setup
  Mesh::HalfedgeHandle heh21 = mesh_.find_halfedge(vhandle[2], vhandle[1]);
  Mesh::HalfedgeHandle heh10 = mesh_.find_halfedge(vhandle[1], vhandle[0]);
  Mesh::HalfedgeHandle heh03 = mesh_.find_halfedge(vhandle[0], vhandle[3]);
  Mesh::HalfedgeHandle heh32 = mesh_.find_halfedge(vhandle[3], vhandle[2]);

  EXPECT_EQ(13, mesh_.property(fprop_int, mesh_.face_handle(heh21))) << "Different Property value";
  EXPECT_EQ(13, mesh_.property(fprop_int, mesh_.face_handle(heh10))) << "Different Property value";
  EXPECT_EQ(17, mesh_.property(fprop_int, mesh_.face_handle(heh03))) << "Different Property value";
  EXPECT_EQ(17, mesh_.property(fprop_int, mesh_.face_handle(heh32))) << "Different Property value";
  EXPECT_TRUE(mesh_.status(mesh_.face_handle(heh21)).tagged()) << "Different internal property value";
  EXPECT_TRUE(mesh_.status(mesh_.face_handle(heh10)).tagged()) << "Different internal property value";
  EXPECT_FALSE(mesh_.status(mesh_.face_handle(heh03)).tagged()) << "Different internal property value";
  EXPECT_FALSE(mesh_.status(mesh_.face_handle(heh32)).tagged()) << "Different internal property value";

  // also test boundary split
  Mesh::EdgeHandle boundary_edge = mesh_.edge_handle(heh10);

  // 1 to 2 split
  mesh_.split_copy(boundary_edge, boundary_vertex);

  Mesh::HalfedgeHandle heh1b = mesh_.find_halfedge(vhandle[1], boundary_vertex);
  Mesh::HalfedgeHandle hehb0 = mesh_.find_halfedge(boundary_vertex, vhandle[0]);

  EXPECT_EQ(13, mesh_.property(fprop_int, mesh_.face_handle(heh1b))) << "Different Property value";
  EXPECT_EQ(13, mesh_.property(fprop_int, mesh_.face_handle(hehb0))) << "Different Property value";
  EXPECT_TRUE(mesh_.status(mesh_.face_handle(heh1b)).tagged()) << "Different internal property value";
  EXPECT_TRUE(mesh_.status(mesh_.face_handle(hehb0)).tagged()) << "Different internal property value";
}

}

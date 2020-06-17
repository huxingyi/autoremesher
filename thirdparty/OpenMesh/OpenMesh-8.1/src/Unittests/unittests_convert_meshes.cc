
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <OpenMesh/Core/Utils/PropertyManager.hh>

namespace {

class OpenMeshConvertTriangleMeshToPoly : public OpenMeshBase {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {    
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
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }

    // Member already defined in OpenMeshBase
    //Mesh mesh_;  
};

class OpenMeshConvertPolyMeshToTriangle : public OpenMeshBasePoly {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
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
          face_vhandles.push_back(vhandle[3]);
          mesh_.add_face(face_vhandles);

          // Test setup:
          //  1 --- 2
          //  |     |
          //  |     |
          //  |     |
          //  0 --- 3
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

/* Checks the converted mesh #faces and #vertices behaviour of adding
 * vertices and faces to a trimesh after the conversion.
 */
TEST_F(OpenMeshConvertTriangleMeshToPoly, VertexFaceCheck) {

  EXPECT_EQ(4u, mesh_.n_vertices() ) << "Wrong number of vertices in TriMesh";
  EXPECT_EQ(2u, mesh_.n_faces() )    << "Wrong number of faces in TriMesh";

  //convert triMesh to PolyMesh
  PolyMesh p = static_cast<PolyMesh>(mesh_);

  // Check setup
  EXPECT_EQ(4u, p.n_vertices() ) << "Wrong number of vertices in PolyMesh";
  EXPECT_EQ(2u, p.n_faces() )    << "Wrong number of faces in PolyMesh";

  //add vertex to original mesh
  Mesh::VertexHandle vhand = mesh_.add_vertex(Mesh::Point(1, 1, 1));

  EXPECT_EQ(5u, mesh_.n_vertices() ) << "Wrong number of vertices in TriMesh";
  EXPECT_EQ(4u, p.n_vertices() ) << "Wrong number of vertices in PolyMesh";

  Mesh::VertexIter it = mesh_.vertices_begin();
  //add face to original mesh
  mesh_.add_face(vhand,(*it),(*++it));

  EXPECT_EQ(3u, mesh_.n_faces() ) << "Wrong number of faces in TriMesh";
  EXPECT_EQ(2u, p.n_faces() ) << "Wrong number of faces in PolyMesh";
}

/* Creates a double property and checks if it works after conversion
 */
TEST_F(OpenMeshConvertTriangleMeshToPoly, VertexPropertyCheckDouble) {

  // Add a double vertex property
  OpenMesh::VPropHandleT<double> doubleHandle;

  EXPECT_FALSE( mesh_.get_property_handle(doubleHandle,"doubleProp") );

  mesh_.add_property(doubleHandle,"doubleProp");

  // Fill property
  double index = 0.0;

  for ( Mesh::VertexIter v_it = mesh_.vertices_begin() ; v_it != mesh_.vertices_end(); ++v_it ) {
    mesh_.property(doubleHandle,*v_it) = index;
    index += 1.0;
  }

  EXPECT_TRUE(mesh_.get_property_handle(doubleHandle,"doubleProp"));

  //convert triMesh to PolyMesh
  PolyMesh p = static_cast<PolyMesh>(mesh_);  

  EXPECT_TRUE(p.get_property_handle(doubleHandle,"doubleProp"));

  // Check if it is ok.
  Mesh::VertexIter v_it = p.vertices_begin();
  EXPECT_EQ( p.property(doubleHandle,*v_it) , 0.0 ) << "Invalid double value for vertex 0";
  ++v_it;

  EXPECT_EQ( p.property(doubleHandle,*v_it) , 1.0 ) << "Invalid double value for vertex 1";
  ++v_it;

  EXPECT_EQ( p.property(doubleHandle,*v_it) , 2.0 ) << "Invalid double value for vertex 2";
  ++v_it;

  EXPECT_EQ( p.property(doubleHandle,*v_it) , 3.0 ) << "Invalid double value for vertex 3";  

  //check if deletion in the original mesh affects the converted one.
  mesh_.get_property_handle(doubleHandle,"doubleProp");
  mesh_.remove_property(doubleHandle);
  EXPECT_FALSE(mesh_.get_property_handle(doubleHandle,"doubleProp"));
  EXPECT_TRUE(p.get_property_handle(doubleHandle,"doubleProp"));

}

/* Checks the converted mesh #faces and #vertices behaviour of adding
 * vertices and faces to a trimesh after the conversion.
 */
TEST_F(OpenMeshConvertPolyMeshToTriangle, VertexFaceCheck) {

  EXPECT_EQ(4u, mesh_.n_vertices() ) << "Wrong number of vertices in PolyMesh";
  EXPECT_EQ(1u, mesh_.n_faces() )    << "Wrong number of faces in PolyMesh";

  //convert PolyMesh to TriMesh
  Mesh p = static_cast<Mesh>(mesh_);

  // Check setup
  EXPECT_EQ(4u, p.n_vertices() ) << "Wrong number of vertices in TriMesh";
  EXPECT_EQ(2u, p.n_faces() )    << "Wrong number of faces in TriMesh";

  //add vertex to original mesh
  Mesh::VertexHandle vhand = mesh_.add_vertex(Mesh::Point(1, 1, 1));

  EXPECT_EQ(5u, mesh_.n_vertices() ) << "Wrong number of vertices in PolyMesh";
  EXPECT_EQ(4u, p.n_vertices() ) << "Wrong number of vertices in TriMesh";

  Mesh::VertexIter it = mesh_.vertices_begin();
  //add face to original mesh
  mesh_.add_face(vhand,(*it),(*++it));

  EXPECT_EQ(2u, mesh_.n_faces() ) << "Wrong number of faces in PolyMesh";
  EXPECT_EQ(2u, p.n_faces() ) << "Wrong number of faces in TriMesh";
}

/* Creates a double property and checks if it works after conversion
 */
TEST_F(OpenMeshConvertPolyMeshToTriangle, VertexPropertyCheckDouble) {

  // Add a double vertex property
  OpenMesh::VPropHandleT<double> doubleHandle;

  EXPECT_FALSE( mesh_.get_property_handle(doubleHandle,"doubleProp") );

  mesh_.add_property(doubleHandle,"doubleProp");

  // Fill property
  double index = 0.0;

  for ( Mesh::VertexIter v_it = mesh_.vertices_begin() ; v_it != mesh_.vertices_end(); ++v_it ) {
    mesh_.property(doubleHandle,*v_it) = index;
    index += 1.0;
  }

  EXPECT_TRUE(mesh_.get_property_handle(doubleHandle,"doubleProp"));

  //convert triMesh to PolyMesh
  Mesh p = static_cast<Mesh>(mesh_);

  EXPECT_TRUE(p.get_property_handle(doubleHandle,"doubleProp"));

  // Check if it is ok.
  Mesh::VertexIter v_it = p.vertices_begin();
  EXPECT_EQ( p.property(doubleHandle,*v_it) , 0.0 ) << "Invalid double value for vertex 0";
  ++v_it;

  EXPECT_EQ( p.property(doubleHandle,*v_it) , 1.0 ) << "Invalid double value for vertex 1";
  ++v_it;

  EXPECT_EQ( p.property(doubleHandle,*v_it) , 2.0 ) << "Invalid double value for vertex 2";
  ++v_it;

  EXPECT_EQ( p.property(doubleHandle,*v_it) , 3.0 ) << "Invalid double value for vertex 3";

  //check if deletion in the original mesh affects the converted one.
  mesh_.get_property_handle(doubleHandle,"doubleProp");
  mesh_.remove_property(doubleHandle);
  EXPECT_FALSE(mesh_.get_property_handle(doubleHandle,"doubleProp"));
  EXPECT_TRUE(p.get_property_handle(doubleHandle,"doubleProp"));

}

/* Creates a double property and checks if it works after conversion
 * especially if edge properties are preserved after triangulation
 */
TEST_F(OpenMeshConvertPolyMeshToTriangle, EdgePropertyCheckDouble) {

  // Add a double vertex property
  OpenMesh::EPropHandleT<double> doubleHandle;

  EXPECT_FALSE( mesh_.get_property_handle(doubleHandle,"doubleProp") );

  mesh_.add_property(doubleHandle,"doubleProp");

  // Fill property
  double index = 0.0;

  for ( Mesh::EdgeIter v_it = mesh_.edges_begin() ; v_it != mesh_.edges_end(); ++v_it ) {
    mesh_.property(doubleHandle,*v_it) = index;
    index += 1.0;
  }

  EXPECT_TRUE(mesh_.get_property_handle(doubleHandle,"doubleProp"));

  //convert triMesh to PolyMesh
  Mesh p = static_cast<Mesh>(mesh_);

  EXPECT_TRUE(p.get_property_handle(doubleHandle,"doubleProp"));

  // Check if it is ok.
  Mesh::EdgeIter v_it = p.edges_begin();

  EXPECT_EQ( p.property(doubleHandle,*v_it) , 0.0 ) << "Invalid double value for vertex 0";
  ++v_it;

  EXPECT_EQ( p.property(doubleHandle,*v_it) , 1.0 ) << "Invalid double value for vertex 1";
  ++v_it;

  EXPECT_EQ( p.property(doubleHandle,*v_it) , 2.0 ) << "Invalid double value for vertex 2";
  ++v_it;

  EXPECT_EQ( p.property(doubleHandle,*v_it) , 3.0 ) << "Invalid double value for vertex 3";
  ++v_it;

  EXPECT_FALSE( p.is_boundary(*v_it)) << "Invalid Edge after triangulation";

  //check if deletion in the original mesh affects the converted one.
  mesh_.get_property_handle(doubleHandle,"doubleProp");
  mesh_.remove_property(doubleHandle);
  EXPECT_FALSE(mesh_.get_property_handle(doubleHandle,"doubleProp"));
  EXPECT_TRUE(p.get_property_handle(doubleHandle,"doubleProp"));

}

}

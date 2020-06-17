
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

namespace {

class OpenMeshProperties : public OpenMeshBase {

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

/* Creates a double property and checks if it works
 */
TEST_F(OpenMeshProperties, VertexPropertyCheckDouble) {

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

  // Check setup
  EXPECT_EQ(4u, mesh_.n_vertices() ) << "Wrong number of vertices";
  EXPECT_EQ(2u, mesh_.n_faces() )    << "Wrong number of faces";

  // Add a double vertex property
  OpenMesh::VPropHandleT<double> doubleHandle;

  EXPECT_FALSE( mesh_.get_property_handle(doubleHandle,"doubleProp") );

  mesh_.add_property(doubleHandle,"doubleProp");

  EXPECT_TRUE(mesh_.get_property_handle(doubleHandle,"doubleProp"));


  // Fill property
  double index = 0.0; 

  for ( Mesh::VertexIter v_it = mesh_.vertices_begin() ; v_it != mesh_.vertices_end(); ++v_it ) {
    mesh_.property(doubleHandle,*v_it) = index;
    index += 1.0;
  }

  // Check if it is ok.
  Mesh::VertexIter v_it = mesh_.vertices_begin();
  EXPECT_EQ( mesh_.property(doubleHandle,*v_it) , 0.0 ) << "Invalid double value for vertex 0";
  ++v_it;

  EXPECT_EQ( mesh_.property(doubleHandle,*v_it) , 1.0 ) << "Invalid double value for vertex 1";
  ++v_it;

  EXPECT_EQ( mesh_.property(doubleHandle,*v_it) , 2.0 ) << "Invalid double value for vertex 2";
  ++v_it;

  EXPECT_EQ( mesh_.property(doubleHandle,*v_it) , 3.0 ) << "Invalid double value for vertex 3";

  // Try to get the stl iterators:
  std::vector<double>::iterator it=mesh_.property(doubleHandle).data_vector().begin();
  std::vector<double>::iterator end=mesh_.property(doubleHandle).data_vector().end();

  EXPECT_EQ( *it , 0.0 ) << "Invalid double value for vertex 0";
  ++it;

  EXPECT_EQ( *it , 1.0 ) << "Invalid double value for vertex 1";
  ++it;

  EXPECT_EQ( *it , 2.0 ) << "Invalid double value for vertex 2";
  ++it;

  EXPECT_EQ( *it , 3.0 ) << "Invalid double value for vertex 3";
  ++it;

  EXPECT_EQ( it, end ) << "End iterator not mathing!";

}

/* Creates a bool property and checks if it works
 */
TEST_F(OpenMeshProperties, VertexPropertyCheckBool) {

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

  // Check setup
  EXPECT_EQ(4u, mesh_.n_vertices() ) << "Wrong number of vertices";
  EXPECT_EQ(2u, mesh_.n_faces() )    << "Wrong number of faces";

  // Add a double vertex property
  OpenMesh::VPropHandleT<bool> boolHandle;

  EXPECT_FALSE( mesh_.get_property_handle(boolHandle,"boolProp") );

  mesh_.add_property(boolHandle,"boolProp");

  EXPECT_TRUE(mesh_.get_property_handle(boolHandle,"boolProp"));

  // Fill property
  bool current = true;

  for ( Mesh::VertexIter v_it = mesh_.vertices_begin() ; v_it != mesh_.vertices_end(); ++v_it ) {
    mesh_.property(boolHandle,*v_it) = current;
    current = !current;
  }

  // Check if it is ok.
  Mesh::VertexIter v_it = mesh_.vertices_begin();
  EXPECT_TRUE( mesh_.property(boolHandle,*v_it) ) << "Invalid bool value for vertex 0";
  ++v_it;

  EXPECT_FALSE( mesh_.property(boolHandle,*v_it) ) << "Invalid bool value for vertex 1";
  ++v_it;

  EXPECT_TRUE( mesh_.property(boolHandle,*v_it) ) << "Invalid bool value for vertex 2";
  ++v_it;

  EXPECT_FALSE( mesh_.property(boolHandle,*v_it) ) << "Invalid bool value for vertex 3";

  // Try to get the stl iterators:
  std::vector<bool>::iterator it=mesh_.property(boolHandle).data_vector().begin();
  std::vector<bool>::iterator end=mesh_.property(boolHandle).data_vector().end();

  EXPECT_TRUE( *it ) << "Invalid bool value for vertex 0";
  ++it;

  EXPECT_FALSE( *it ) << "Invalid bool value for vertex 1";
  ++it;
  
  EXPECT_TRUE( *it ) << "Invalid bool value for vertex 2";
  ++it;
  
  EXPECT_FALSE( *it ) << "Invalid bool value for vertex 3";
  ++it;

  EXPECT_EQ( it, end ) << "End iterator not mathing!";

}

/* Creates an int property and checks if it the copy operation works
 */
TEST_F(OpenMeshProperties, VertexPropertyCopypropertiesInt) {

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

  // Check setup
  EXPECT_EQ(4u, mesh_.n_vertices() ) << "Wrong number of vertices";
  EXPECT_EQ(2u, mesh_.n_faces() )    << "Wrong number of faces";

  // Add a double vertex property
  OpenMesh::VPropHandleT<int> intHandle;

  EXPECT_FALSE( mesh_.get_property_handle(intHandle,"intProp") );

  mesh_.add_property(intHandle,"intProp");

  EXPECT_TRUE(mesh_.get_property_handle(intHandle,"intProp"));

  // Fill property
  for ( Mesh::VertexIter v_it = mesh_.vertices_begin() ; v_it != mesh_.vertices_end(); ++v_it ) {
    mesh_.property(intHandle,*v_it) = v_it->idx();
  }

  // Check if property it is ok.
  Mesh::VertexIter v_it = mesh_.vertices_begin();
  EXPECT_EQ( 0, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 0";
  ++v_it;

  EXPECT_EQ( 1, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 1";
  ++v_it;

  EXPECT_EQ( 2, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 2";
  ++v_it;

  EXPECT_EQ( 3, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 3";

  // Check vertex positions
  v_it = mesh_.vertices_begin();

  EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 0";
  EXPECT_EQ( 0, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 0";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 0";
  ++v_it;

  EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 1";
  EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 1";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 1";
  ++v_it;

  EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 2";
  EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 2";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 2";
  ++v_it;

  EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 3";
  EXPECT_EQ( 0, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 3";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 3";

  //===========================================================
  // Copy from vertex 1 to 0, with skipping build in properties
  //===========================================================
  mesh_.copy_all_properties(vhandle[1], vhandle[0]);

  // Check vertex positions
  v_it = mesh_.vertices_begin();

  EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 0 after copy";
  EXPECT_EQ( 0, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 0 after copy";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 0 after copy";
  ++v_it;

  EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 1 after copy";
  EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 1 after copy";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 1 after copy";
  ++v_it;

  EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 2 after copy";
  EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 2 after copy";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 2 after copy";
  ++v_it;

  EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 3 after copy";
  EXPECT_EQ( 0, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 3 after copy";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 3 after copy";

  v_it = mesh_.vertices_begin();
  EXPECT_EQ( 1, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 0 after copy"; ++v_it;
  EXPECT_EQ( 1, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 1 after copy"; ++v_it;
  EXPECT_EQ( 2, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 2 after copy"; ++v_it;
  EXPECT_EQ( 3, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 3 after copy";

  //===========================================================
  // Copy from vertex 2 to 3, including build in properties
  //===========================================================
  mesh_.copy_all_properties(vhandle[2], vhandle[3], true);

  // Check vertex positions
  v_it = mesh_.vertices_begin();

  EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 0 after copy";
  EXPECT_EQ( 0, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 0 after copy";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 0 after copy";
  ++v_it;

  EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 1 after copy";
  EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 1 after copy";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 1 after copy";
  ++v_it;

  EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 2 after copy";
  EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 2 after copy";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 2 after copy";
  ++v_it;

  EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 3 after copy";
  EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 3 after copy";
  EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 3 after copy";

  v_it = mesh_.vertices_begin();
  EXPECT_EQ( 1, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 0 after copy"; ++v_it;
  EXPECT_EQ( 1, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 1 after copy"; ++v_it;
  EXPECT_EQ( 2, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 2 after copy"; ++v_it;
  EXPECT_EQ( 2, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 3 after copy";


}

/*
 * Checking for deleted flags of halfedge and edge handles
 * 
 * Checks if after deleting a face, all halfedges and edges are arked as deleted as well
*/
TEST_F(OpenMeshProperties, CheckStatusPropertiesHalfedgeEdgeAllDeleted) {

  mesh_.clear();

  mesh_.request_vertex_status();
  mesh_.request_face_status();
  mesh_.request_halfedge_status();
  mesh_.request_edge_status();

  // Define positions
  Mesh::Point p1 = Mesh::Point(0, 0, 0);
  Mesh::Point p2 = Mesh::Point(0, 1, 0);
  Mesh::Point p3 = Mesh::Point(1, 1, 0);
  Mesh::Point p4 = Mesh::Point(0, 0, 1);
  
  // Add some vertices
  Mesh::VertexHandle vh1 = mesh_.add_vertex(p1);
  Mesh::VertexHandle vh2 = mesh_.add_vertex(p2);
  Mesh::VertexHandle vh3 = mesh_.add_vertex(p3);
  Mesh::VertexHandle vh4 = mesh_.add_vertex(p4);

  // Add some faces
  Mesh::FaceHandle f1 = mesh_.add_face(vh1,vh3,vh2);
  Mesh::FaceHandle f2 = mesh_.add_face(vh1,vh2,vh4);
  Mesh::FaceHandle f3 = mesh_.add_face(vh2,vh3,vh4);
  Mesh::FaceHandle f4 = mesh_.add_face(vh3,vh1,vh4);

  // delete all faces
  mesh_.delete_face(f1);
  mesh_.delete_face(f2);
  mesh_.delete_face(f3);
  mesh_.delete_face(f4);

  for( Mesh::ConstHalfedgeIter he_it = mesh_.halfedges_begin(); he_it != mesh_.halfedges_end(); ++he_it)
  {
      EXPECT_TRUE( mesh_.status(mesh_.edge_handle(*he_it)).deleted()  ) << "Edge not deleted";
      EXPECT_TRUE( mesh_.status(*he_it).deleted()                     ) << "Halfedge not deleted";
  }

}

/*
 * Copy properties from one mesh to another
 *
*/
TEST_F(OpenMeshProperties, CopyAllPropertiesVertexAfterRemoveOfProperty) {

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

   // Check setup
   EXPECT_EQ(4u, mesh_.n_vertices() ) << "Wrong number of vertices";
   EXPECT_EQ(2u, mesh_.n_faces() )    << "Wrong number of faces";

   // Add a double vertex property
   OpenMesh::VPropHandleT<double> doubleHandle;

   EXPECT_FALSE( mesh_.get_property_handle(doubleHandle,"doubleProp") );

   mesh_.add_property(doubleHandle,"doubleProp");

   EXPECT_TRUE(mesh_.get_property_handle(doubleHandle,"doubleProp"));

   // Add a double vertex property
   OpenMesh::VPropHandleT<int> intHandle;

   EXPECT_FALSE( mesh_.get_property_handle(intHandle,"intProp") );

   mesh_.add_property(intHandle,"intProp");

   EXPECT_TRUE(mesh_.get_property_handle(intHandle,"intProp"));

   // Now remove the double property again.
   mesh_.remove_property(doubleHandle);

   // Fill int property
   for ( Mesh::VertexIter v_it = mesh_.vertices_begin() ; v_it != mesh_.vertices_end(); ++v_it ) {
     mesh_.property(intHandle,*v_it) = v_it->idx();
   }

   // Check if property it is ok.
   Mesh::VertexIter v_it = mesh_.vertices_begin();
   EXPECT_EQ( 0, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 0";
   ++v_it;

   EXPECT_EQ( 1, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 1";
   ++v_it;

   EXPECT_EQ( 2, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 2";
   ++v_it;

   EXPECT_EQ( 3, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 3";

   // Check vertex positions
   v_it = mesh_.vertices_begin();

   EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 0";
   EXPECT_EQ( 0, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 0";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 0";
   ++v_it;

   EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 1";
   EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 1";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 1";
   ++v_it;

   EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 2";
   EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 2";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 2";
   ++v_it;

   EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 3";
   EXPECT_EQ( 0, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 3";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 3";

   //===========================================================
   // Copy from vertex 1 to 0, with skipping build in properties
   //===========================================================
   mesh_.copy_all_properties(vhandle[1], vhandle[0]);

   // Check vertex positions
   v_it = mesh_.vertices_begin();

   EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 0 after copy";
   EXPECT_EQ( 0, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 0 after copy";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 0 after copy";
   ++v_it;

   EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 1 after copy";
   EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 1 after copy";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 1 after copy";
   ++v_it;

   EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 2 after copy";
   EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 2 after copy";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 2 after copy";
   ++v_it;

   EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 3 after copy";
   EXPECT_EQ( 0, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 3 after copy";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 3 after copy";

   v_it = mesh_.vertices_begin();
   EXPECT_EQ( 1, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 0 after copy"; ++v_it;
   EXPECT_EQ( 1, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 1 after copy"; ++v_it;
   EXPECT_EQ( 2, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 2 after copy"; ++v_it;
   EXPECT_EQ( 3, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 3 after copy";

   //===========================================================
   // Copy from vertex 2 to 3, including build in properties
   //===========================================================
   mesh_.copy_all_properties(vhandle[2], vhandle[3], true);

   // Check vertex positions
   v_it = mesh_.vertices_begin();

   EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 0 after copy";
   EXPECT_EQ( 0, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 0 after copy";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 0 after copy";
   ++v_it;

   EXPECT_EQ( 0, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 1 after copy";
   EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 1 after copy";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 1 after copy";
   ++v_it;

   EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 2 after copy";
   EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 2 after copy";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 2 after copy";
   ++v_it;

   EXPECT_EQ( 1, mesh_.point(*v_it)[0] ) << "Invalid x position for vertex 3 after copy";
   EXPECT_EQ( 1, mesh_.point(*v_it)[1] ) << "Invalid y position for vertex 3 after copy";
   EXPECT_EQ( 0, mesh_.point(*v_it)[2] ) << "Invalid z position for vertex 3 after copy";

   v_it = mesh_.vertices_begin();
   EXPECT_EQ( 1, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 0 after copy"; ++v_it;
   EXPECT_EQ( 1, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 1 after copy"; ++v_it;
   EXPECT_EQ( 2, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 2 after copy"; ++v_it;
   EXPECT_EQ( 2, mesh_.property(intHandle,*v_it) ) << "Invalid int value for vertex 3 after copy";



}

/* Creates a double and int properties and check if we can iterate across them
 */
TEST_F(OpenMeshProperties, PropertyIterators ) {

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

  // Check setup
  EXPECT_EQ(4u, mesh_.n_vertices() ) << "Wrong number of vertices";
  EXPECT_EQ(2u, mesh_.n_faces() )    << "Wrong number of faces";


  // Add vertex properties
  OpenMesh::VPropHandleT<double> doubleHandleV;
  OpenMesh::VPropHandleT<double> intHandleV;

  EXPECT_FALSE( mesh_.get_property_handle(doubleHandleV,"doublePropV") );
  EXPECT_FALSE( mesh_.get_property_handle(intHandleV,"intPropV") );

  mesh_.add_property(doubleHandleV,"doublePropV");
  mesh_.add_property(intHandleV,"intPropV");

  EXPECT_TRUE(mesh_.get_property_handle(doubleHandleV,"doublePropV"));
  EXPECT_TRUE(mesh_.get_property_handle(intHandleV,"intPropV"));

  // Add a double Edge properties
  OpenMesh::EPropHandleT<double> doubleHandleE;
  OpenMesh::EPropHandleT<double> intHandleE;

  EXPECT_FALSE( mesh_.get_property_handle(doubleHandleE,"doublePropE") );
  EXPECT_FALSE( mesh_.get_property_handle(intHandleE,"intPropE") );

  mesh_.add_property(doubleHandleE,"doublePropE");
  mesh_.add_property(intHandleE,"intPropE");

  EXPECT_TRUE(mesh_.get_property_handle(doubleHandleE,"doublePropE"));
  EXPECT_TRUE(mesh_.get_property_handle(intHandleE,"intPropE"));


  // Add Face properties
  OpenMesh::FPropHandleT<double> doubleHandleF;
  OpenMesh::FPropHandleT<double> intHandleF;

  EXPECT_FALSE( mesh_.get_property_handle(doubleHandleF,"doublePropF") );
  EXPECT_FALSE( mesh_.get_property_handle(intHandleF,"intPropF") );

  mesh_.add_property(doubleHandleF,"doublePropF");
  mesh_.add_property(intHandleF,"intPropF");

  EXPECT_TRUE(mesh_.get_property_handle(doubleHandleF,"doublePropF"));
  EXPECT_TRUE(mesh_.get_property_handle(intHandleF,"intPropF"));




  unsigned int vertex_props = 0;
    for (Mesh::prop_iterator vprop_it = mesh_.vprops_begin() ; vprop_it != mesh_.vprops_end(); ++vprop_it) {
      switch (vertex_props) {
        case 0:
          EXPECT_EQ ("v:points",(*vprop_it)->name()) << "Wrong Vertex property name";
          break;
        case 1:
          EXPECT_EQ ("<vprop>",(*vprop_it)->name()) << "Wrong Vertex property name";
          break;
        case 2:
          EXPECT_EQ ("doublePropV",(*vprop_it)->name()) << "Wrong Vertex property name";
          break;
        case 3:
          EXPECT_EQ ("intPropV",(*vprop_it)->name()) << "Wrong Vertex property name";
          break;
        default:
          EXPECT_EQ (4u , vertex_props);
          break;
      }
      ++vertex_props;
    }

    EXPECT_EQ (4u,vertex_props) << "Wrong number of vertex properties";



  unsigned int edge_props = 0;
  for (Mesh::prop_iterator eprop_it = mesh_.eprops_begin() ; eprop_it != mesh_.eprops_end(); ++eprop_it) {
    switch (edge_props) {
      case 0:
        EXPECT_EQ ("<eprop>",(*eprop_it)->name()) << "Wrong Edge property name";
        break;
      case 1:
        EXPECT_EQ ("doublePropE",(*eprop_it)->name()) << "Wrong Edge property name";
        break;
      case 2:
        EXPECT_EQ ("intPropE",(*eprop_it)->name()) << "Wrong Edge property name";
        break;
      default:
        EXPECT_EQ (4u , edge_props);
        break;
    }
    ++edge_props;
  }

  EXPECT_EQ (3u,edge_props) << "Wrong number of edge properties";



  unsigned int face_props = 0;
  for (Mesh::prop_iterator prop_it = mesh_.fprops_begin() ; prop_it != mesh_.fprops_end(); ++prop_it) {
    switch (face_props) {
      case 0:
        EXPECT_EQ ("<fprop>",(*prop_it)->name()) << "Wrong Face property name";
        break;
      case 1:
        EXPECT_EQ ("doublePropF",(*prop_it)->name()) << "Wrong Face property name";
        break;
      case 2:
        EXPECT_EQ ("intPropF",(*prop_it)->name()) << "Wrong Face property name";
        break;
      default:
        EXPECT_EQ (4u , face_props);
        break;
    }
    ++face_props;
  }

  EXPECT_EQ (3u,face_props) << "Wrong number of face properties";


}


TEST_F(OpenMeshProperties, MeshAssignment ) {

  mesh_.clear();
  mesh_.add_vertex(Mesh::Point());

  auto copy = mesh_;

  copy.request_vertex_status();
  copy.request_vertex_normals();
  copy.request_vertex_colors();
  copy.request_vertex_texcoords1D();
  copy.request_vertex_texcoords2D();
  copy.request_vertex_texcoords3D();
  copy.request_halfedge_status();
  copy.request_halfedge_texcoords1D();
  copy.request_halfedge_texcoords2D();
  copy.request_halfedge_texcoords3D();
  copy.request_edge_status();
  copy.request_edge_colors();
  copy.request_halfedge_normals();
  copy.request_halfedge_colors();
  copy.request_face_status();
  copy.request_face_normals();
  copy.request_face_colors();
  copy.request_face_texture_index();

  EXPECT_TRUE(copy.has_vertex_status());
  EXPECT_TRUE(copy.has_vertex_normals());
  EXPECT_TRUE(copy.has_vertex_colors());
  EXPECT_TRUE(copy.has_vertex_texcoords1D());
  EXPECT_TRUE(copy.has_vertex_texcoords2D());
  EXPECT_TRUE(copy.has_vertex_texcoords3D());
  EXPECT_TRUE(copy.has_halfedge_status());
  EXPECT_TRUE(copy.has_halfedge_texcoords1D());
  EXPECT_TRUE(copy.has_halfedge_texcoords2D());
  EXPECT_TRUE(copy.has_halfedge_texcoords3D());
  EXPECT_TRUE(copy.has_edge_status());
  EXPECT_TRUE(copy.has_edge_colors());
  EXPECT_TRUE(copy.has_halfedge_normals());
  EXPECT_TRUE(copy.has_halfedge_colors());
  EXPECT_TRUE(copy.has_face_status());
  EXPECT_TRUE(copy.has_face_normals());
  EXPECT_TRUE(copy.has_face_colors());
  EXPECT_TRUE(copy.has_face_texture_index());

  copy.assign(mesh_, true);

  EXPECT_FALSE(copy.has_vertex_status());
  EXPECT_FALSE(copy.has_vertex_normals());
  EXPECT_FALSE(copy.has_vertex_colors());
  EXPECT_FALSE(copy.has_vertex_texcoords1D());
  EXPECT_FALSE(copy.has_vertex_texcoords2D());
  EXPECT_FALSE(copy.has_vertex_texcoords3D());
  EXPECT_FALSE(copy.has_halfedge_status());
  EXPECT_FALSE(copy.has_halfedge_texcoords1D());
  EXPECT_FALSE(copy.has_halfedge_texcoords2D());
  EXPECT_FALSE(copy.has_halfedge_texcoords3D());
  EXPECT_FALSE(copy.has_edge_status());
  EXPECT_FALSE(copy.has_edge_colors());
  EXPECT_FALSE(copy.has_halfedge_normals());
  EXPECT_FALSE(copy.has_halfedge_colors());
  EXPECT_FALSE(copy.has_face_status());
  EXPECT_FALSE(copy.has_face_normals());
  EXPECT_FALSE(copy.has_face_colors());
  EXPECT_FALSE(copy.has_face_texture_index());

  copy.request_vertex_status();
  copy.request_vertex_normals();
  copy.request_vertex_colors();
  copy.request_vertex_texcoords1D();
  copy.request_vertex_texcoords2D();
  copy.request_vertex_texcoords3D();
  copy.request_halfedge_status();
  copy.request_halfedge_texcoords1D();
  copy.request_halfedge_texcoords2D();
  copy.request_halfedge_texcoords3D();
  copy.request_edge_status();
  copy.request_edge_colors();
  copy.request_halfedge_normals();
  copy.request_halfedge_colors();
  copy.request_face_status();
  copy.request_face_normals();
  copy.request_face_colors();
  copy.request_face_texture_index();

  EXPECT_TRUE(copy.has_vertex_status())        << "Mesh has no vertex status even though they have been requested";
  EXPECT_TRUE(copy.has_vertex_normals())       << "Mesh has no vertex normals even though they have been requested";
  EXPECT_TRUE(copy.has_vertex_colors())        << "Mesh has no vertex colors even though they have been requested";
  EXPECT_TRUE(copy.has_vertex_texcoords1D())   << "Mesh has no vertex texcoord1D even though they have been requested";
  EXPECT_TRUE(copy.has_vertex_texcoords2D())   << "Mesh has no vertex texcoord2D even though they have been requested";
  EXPECT_TRUE(copy.has_vertex_texcoords3D())   << "Mesh has no vertex texcoord3D even though they have been requested";
  EXPECT_TRUE(copy.has_halfedge_status())      << "Mesh has no halfedge status even though they have been requested";
  EXPECT_TRUE(copy.has_halfedge_texcoords1D()) << "Mesh has no halfedge texcoords1D even though they have been requested";
  EXPECT_TRUE(copy.has_halfedge_texcoords2D()) << "Mesh has no halfedge texcoords2D even though they have been requested";
  EXPECT_TRUE(copy.has_halfedge_texcoords3D()) << "Mesh has no halfedge texcoords3D even though they have been requested";
  EXPECT_TRUE(copy.has_edge_status())          << "Mesh has no edge status even though they have been requested";
  EXPECT_TRUE(copy.has_edge_colors())          << "Mesh has no edge colors even though they have been requested";
  EXPECT_TRUE(copy.has_halfedge_normals())     << "Mesh has no halfedge normals even though they have been requested";
  EXPECT_TRUE(copy.has_halfedge_colors())      << "Mesh has no halfedge colors even though they have been requested";
  EXPECT_TRUE(copy.has_face_status())          << "Mesh has no face status even though they have been requested";
  EXPECT_TRUE(copy.has_face_normals())         << "Mesh has no face normals even though they have been requested";
  EXPECT_TRUE(copy.has_face_colors())          << "Mesh has no face colors even though they have been requested";
  EXPECT_TRUE(copy.has_face_texture_index())   << "Mesh has no face texture index even though they have been requested";

}


}

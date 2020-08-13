#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

namespace {

class OpenMeshNormals : public OpenMeshBase {

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
 * Collapsing a tetrahedron
 */
TEST_F(OpenMeshNormals, NormalCalculations) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0, 0, 1));

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

  // ===============================================
  // Setup complete
  // ===============================================


  // Check one Request only vertex normals
  // Face normals are required for vertex and halfedge normals, so 
  // that prevent access to non existing properties are in place

  mesh_.request_vertex_normals();
  mesh_.request_halfedge_normals();

  // Check blocks
  mesh_.update_normals();

  // Request required face normals 
  mesh_.request_face_normals();

  // Automatically compute all normals
  // As only vertex normals are requested and no face normals, this will compute nothing.
  mesh_.update_normals();

  // Face normals alone
  mesh_.update_face_normals();

  // Vertex normals alone (require valid face normals)
  mesh_.update_vertex_normals();

  // Halfedge normals alone (require valid face normals)
  mesh_.update_halfedge_normals();

}
}

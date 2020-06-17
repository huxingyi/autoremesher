#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <iostream>

struct CustomTraitsVec2i : OpenMesh::DefaultTraits
{
    typedef OpenMesh::Vec2i Point;
};


typedef OpenMesh::TriMesh_ArrayKernelT<CustomTraitsVec2i> TriMeshVec2i;


/*
 * OpenMesh Triangular with Vec2i
 */

class OpenMeshBaseTriVec2i : public testing::Test {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {

            // Do some initial stuff with the member data here...
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }

    // This member will be accessible in all tests
   TriMeshVec2i mesh_;
};


namespace {

/*
 * ====================================================================
 * Define tests below
 * ====================================================================
 */

/*
 * Checking for feature edges based on angle
 */
TEST_F(OpenMeshBaseTriVec2i, Instance_Vec2i_Mesh) {

  mesh_.clear();

  // Add some vertices
  TriMeshVec2i::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(TriMeshVec2i::Point(0, 0));
  vhandle[1] = mesh_.add_vertex(TriMeshVec2i::Point(0, 1));
  vhandle[2] = mesh_.add_vertex(TriMeshVec2i::Point(1, 1));

  // Add face
  std::vector<TriMeshVec2i::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  // ===============================================
  // Setup complete
  // ===============================================

  // Check one Request only vertex normals
  // Face normals are required for vertex and halfedge normals, so 
  // that prevent access to non existing properties are in place

  mesh_.request_vertex_normals();
  mesh_.request_halfedge_normals();
  mesh_.request_face_normals();

}

}

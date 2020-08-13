
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <iostream>

namespace {

class OpenMeshSplitCopyTriangleMesh : public OpenMeshBase {

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

class OpenMeshSplitCopyPolyMesh : public OpenMeshBasePoly {

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

/* splits a face that has a property in a triangle mesh with split_copy
 * the property should be copied to the new faces
 */
TEST_F(OpenMeshSplitCopyTriangleMesh, SplitCopyTriangleMesh) {

  mesh_.clear();

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

  // Test setup:
  //  1 === 2
  //  |   /
  //  |  /
  //  | /
  //  0

  // set property
  OpenMesh::FPropHandleT<int> fprop_int;
  mesh_.add_property(fprop_int);
  mesh_.property(fprop_int, fh) = 999;

  // split face with new vertex
  mesh_.split_copy(fh, vhandle[3]);

  // Check setup
  Mesh::FaceIter f_it = mesh_.faces_begin();
  Mesh::FaceIter f_end = mesh_.faces_end();
  for (; f_it != f_end; ++f_it)
    EXPECT_EQ(999, mesh_.property(fprop_int, *f_it)) << "Different Property value";
}

/* splits a face that has a property in a poly mesh with split_copy
 * the property should be copied to the new faces
 */
TEST_F(OpenMeshSplitCopyPolyMesh, SplitCopyPolymesh) {

  mesh_.clear();

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

  PolyMesh::FaceHandle fh = mesh_.add_face(face_vhandles);

  // Test setup:
  //  1 === 2
  //  |     |
  //  |     |
  //  |     |
  //  0 === 3

  // set property
  OpenMesh::FPropHandleT<int> fprop_int;
  mesh_.add_property(fprop_int);
  mesh_.property(fprop_int, fh) = 999;

  // split face with new vertex
  mesh_.split_copy(fh, vhandle[4]);

  // Check setup
  PolyMesh::FaceIter f_it = mesh_.faces_begin();
  PolyMesh::FaceIter f_end = mesh_.faces_end();
  for (; f_it != f_end; ++f_it)
    EXPECT_EQ(999, mesh_.property(fprop_int, *f_it)) << "Different Property value";

}
}

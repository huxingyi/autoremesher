
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <iostream>

namespace {

class OpenMeshSplitTriangleMesh : public OpenMeshBase {

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
TEST_F(OpenMeshSplitTriangleMesh, Split_Triangle_Mesh_1_4_center) {

  mesh_.clear();

  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_face_status();

  // Add some vertices
  Mesh::VertexHandle vhandle[6];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(3, 0, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(4, 1, 0));
  vhandle[5] = mesh_.add_vertex(Mesh::Point(2,-1, 0));

  // Add three faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  Mesh::FaceHandle to_split = mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  /* Test setup:
   *
   * edge x => halfedge x/x+1
   * i.e. edge 0 => halfedge 0/1
   *
   * 0 --4--- 2 ------ 4
   *  \      / \      /
   *   0  0 2   6  2 /
   *    \  /  1  \  /
   *     1 ---8--- 3
   *      \       /
   *       \  3  /
   *        \   /
   *         \ /
   *          5
   */

  EXPECT_EQ(4u, mesh_.n_faces());

  // split face in center (non-boundary case)
  mesh_.split(to_split);

  mesh_.garbage_collection();

  EXPECT_EQ(10u, mesh_.n_faces());

}

/* splits a face that has a property in a triangle mesh with split_copy
 * the property should be copied to the new faces
 */
TEST_F(OpenMeshSplitTriangleMesh, Split_Triangle_Mesh_1_4_boundary) {

  mesh_.clear();

  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_face_status();

  // Add some vertices
  Mesh::VertexHandle vhandle[6];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(3, 0, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(4, 1, 0));
  vhandle[5] = mesh_.add_vertex(Mesh::Point(2,-1, 0));

  // Add three faces
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  Mesh::FaceHandle to_split = mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[4]);
  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);

  /* Test setup:
   *
   * edge x => halfedge x/x+1
   * i.e. edge 0 => halfedge 0/1
   *
   * 0 --4--- 2 ------ 4
   *  \      / \      /
   *   0  0 2   6  2 /
   *    \  /  1  \  /
   *     1 ---8--- 3
   *      \       /
   *       \  3  /
   *        \   /
   *         \ /
   *          5
   */

  EXPECT_EQ(4u, mesh_.n_faces());

  // split face at boundary
  mesh_.split(to_split);

  mesh_.garbage_collection();

  EXPECT_EQ(8u, mesh_.n_faces());

}


}




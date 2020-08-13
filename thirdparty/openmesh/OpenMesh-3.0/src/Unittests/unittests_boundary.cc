#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <iostream>

namespace {
    
class OpenMeshBoundaryTriangleMesh : public OpenMeshBase {

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

/* Checks vertices, if they are boundary vertices
 */
TEST_F(OpenMeshBoundaryTriangleMesh, TestBoundaryVertex) {

  mesh_.clear();

    // Add some vertices
    Mesh::VertexHandle vhandle[7];

    vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
    vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
    vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
    vhandle[3] = mesh_.add_vertex(Mesh::Point(0,-1, 0));
    vhandle[4] = mesh_.add_vertex(Mesh::Point(2,-1, 0));
    vhandle[5] = mesh_.add_vertex(Mesh::Point(3, 0, 0));

    // Single point
    vhandle[6] = mesh_.add_vertex(Mesh::Point(0,-2, 0));

    // Add two faces
    std::vector<Mesh::VertexHandle> face_vhandles;

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    mesh_.add_face(face_vhandles);

    face_vhandles.clear();

    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[4]);
    mesh_.add_face(face_vhandles);

    face_vhandles.clear();

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[1]);
    mesh_.add_face(face_vhandles);

    face_vhandles.clear();

    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[4]);
    mesh_.add_face(face_vhandles);

    face_vhandles.clear();

    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[4]);
    mesh_.add_face(face_vhandles);

    /* Test setup:
        0 ==== 2
        |\  0 /|\
        | \  / | \
        |2  1 3|4 5
        | /  \ | /
        |/  1 \|/
        3 ==== 4

        Vertex 6 single
        */


    // Check for boundary vertices
    EXPECT_TRUE ( mesh_.is_boundary( vhandle[0] ) ) << "Vertex 0 is not boundary!";
    EXPECT_FALSE( mesh_.is_boundary( vhandle[1] ) ) << "Vertex 1 is boundary!";
    EXPECT_TRUE ( mesh_.is_boundary( vhandle[2] ) ) << "Vertex 2 is not boundary!";
    EXPECT_TRUE ( mesh_.is_boundary( vhandle[3] ) ) << "Vertex 3 is not boundary!";
    EXPECT_TRUE ( mesh_.is_boundary( vhandle[4] ) ) << "Vertex 4 is not boundary!";
    EXPECT_TRUE ( mesh_.is_boundary( vhandle[5] ) ) << "Vertex 5 is not boundary!";

    EXPECT_TRUE ( mesh_.is_boundary( vhandle[6] ) ) << "Singular Vertex 6 is not boundary!";


}

/* Checks faces, if they are boundary faces
 */
TEST_F(OpenMeshBoundaryTriangleMesh, TestBoundaryFace) {

  mesh_.clear();

    // Add some vertices
    Mesh::VertexHandle vhandle[7];

    vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
    vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
    vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
    vhandle[3] = mesh_.add_vertex(Mesh::Point(0,-1, 0));
    vhandle[4] = mesh_.add_vertex(Mesh::Point(2,-1, 0));
    vhandle[5] = mesh_.add_vertex(Mesh::Point(3, 0, 0));

    // Single point
    vhandle[6] = mesh_.add_vertex(Mesh::Point(0,-2, 0));

    // Add two faces
    std::vector<Mesh::VertexHandle> face_vhandles;
    std::vector<Mesh::FaceHandle>   face_handles;

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    face_handles.push_back(mesh_.add_face(face_vhandles));

    face_vhandles.clear();

    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[4]);
    face_handles.push_back(mesh_.add_face(face_vhandles));

    face_vhandles.clear();

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[1]);
    face_handles.push_back(mesh_.add_face(face_vhandles));

    face_vhandles.clear();

    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[4]);
    face_handles.push_back(mesh_.add_face(face_vhandles));

    face_vhandles.clear();

    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[4]);
    face_handles.push_back(mesh_.add_face(face_vhandles));

    /* Test setup:
        0 ==== 2
        |\  0 /|\
        | \  / | \
        |2  1 3|4 5
        | /  \ | /
        |/  1 \|/
        3 ==== 4

        Vertex 6 single
        */


    // Check the boundary faces
    EXPECT_TRUE ( mesh_.is_boundary( face_handles[0] ) ) << "Face 0 is not boundary!";
    EXPECT_TRUE ( mesh_.is_boundary( face_handles[1] ) ) << "Face 1 is not boundary!";
    EXPECT_TRUE ( mesh_.is_boundary( face_handles[2] ) ) << "Face 2 is not boundary!";
    EXPECT_FALSE( mesh_.is_boundary( face_handles[3] ) ) << "Face 3 is boundary!";
    EXPECT_TRUE ( mesh_.is_boundary( face_handles[4] ) ) << "Face 4 is not boundary!";

}
}

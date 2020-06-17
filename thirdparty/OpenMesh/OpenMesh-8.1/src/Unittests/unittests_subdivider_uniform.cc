
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/ModifiedButterFlyT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/CatmullClarkT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/LoopT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/Sqrt3T.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/MidpointT.hh>

namespace {

class OpenMeshSubdividerUniform_Poly : public OpenMeshBasePoly {
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

class OpenMeshSubdividerUniform_Triangle : public OpenMeshBase {
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

TEST_F(OpenMeshSubdividerUniform_Triangle, Subdivider_Sqrt3) {
    mesh_.clear();

    // Add some vertices
    Mesh::VertexHandle vhandle[9];

    vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
    vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
    vhandle[2] = mesh_.add_vertex(Mesh::Point(0, 2, 0));
    vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
    vhandle[4] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
    vhandle[5] = mesh_.add_vertex(Mesh::Point(1, 2, 0));
    vhandle[6] = mesh_.add_vertex(Mesh::Point(2, 0, 0));
    vhandle[7] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
    vhandle[8] = mesh_.add_vertex(Mesh::Point(2, 2, 0));

    // Add eight faces
    std::vector<Mesh::VertexHandle> face_vhandles;

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[3]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[6]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[8]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[8]);

    mesh_.add_face(face_vhandles);

    // Test setup:
    //  6 === 7 === 8
    //  |   / |   / |
    //  |  /  |  /  |
    //  | /   | /   |
    //  3 === 4 === 5
    //  |   / | \   |
    //  |  /  |  \  |
    //  | /   |   \ |
    //  0 === 1 === 2

    // Initialize subdivider
    OpenMesh::Subdivider::Uniform::Sqrt3T<Mesh> sqrt3;

    // Check setup
    EXPECT_EQ(9u, mesh_.n_vertices() ) << "Wrong number of vertices";
    EXPECT_EQ(8u, mesh_.n_faces() )    << "Wrong number of faces";

    // Execute 3 subdivision steps
    sqrt3.attach(mesh_);
    sqrt3( 3 );
    sqrt3.detach();

    // Check setup
    EXPECT_EQ(121u, mesh_.n_vertices() ) << "Wrong number of vertices after subdivision with sqrt3";
    EXPECT_EQ(216u, mesh_.n_faces() )    << "Wrong number of faces after subdivision with sqrt3";
}


TEST_F(OpenMeshSubdividerUniform_Triangle, Subdivider_Sqrt3_delete_vertex) {

  for (bool collect_garbage : { false, true })
  {
    mesh_.clear();

    // Request status flags to use delete and garbage collection
    mesh_.request_vertex_status();
    mesh_.request_halfedge_status();
    mesh_.request_edge_status();
    mesh_.request_face_status();

    // Add some vertices
    Mesh::VertexHandle vhandle[9];

    vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
    vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
    vhandle[2] = mesh_.add_vertex(Mesh::Point(0, 2, 0));
    vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
    vhandle[4] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
    vhandle[5] = mesh_.add_vertex(Mesh::Point(1, 2, 0));
    vhandle[6] = mesh_.add_vertex(Mesh::Point(2, 0, 0));
    vhandle[7] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
    vhandle[8] = mesh_.add_vertex(Mesh::Point(2, 2, 0));

    // Add eight faces
    std::vector<Mesh::VertexHandle> face_vhandles;

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[3]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[6]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[8]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[8]);

    mesh_.add_face(face_vhandles);

    // Test setup:
    //  6 === 7 === 8
    //  |   / |   / |
    //  |  /  |  /  |
    //  | /   | /   |
    //  3 === 4 === 5
    //  |   / | \   |
    //  |  /  |  \  |
    //  | /   |   \ |
    //  0 === 1 === 2

    // Delete one vertex
    mesh_.delete_vertex(vhandle[1]);
    // Check setup
    if (collect_garbage)
    {
      mesh_.garbage_collection();
      EXPECT_EQ(8u, mesh_.n_vertices() ) << "Wrong number of vertices";
      EXPECT_EQ(6u, mesh_.n_faces() )    << "Wrong number of faces";
    }
    else
    {
      EXPECT_EQ(9u, mesh_.n_vertices() ) << "Wrong number of vertices";
      EXPECT_EQ(8u, mesh_.n_faces() )    << "Wrong number of faces";
    }


    // Initialize subdivider
    OpenMesh::Subdivider::Uniform::Sqrt3T<Mesh> sqrt3;


    // Execute 3 subdivision steps
    sqrt3.attach(mesh_);
    sqrt3( 3 );
    sqrt3.detach();

    if (!collect_garbage)
      mesh_.garbage_collection(); // if we did not collect garbage before, do so now

    // Check setup
    EXPECT_EQ(94u, mesh_.n_vertices() ) << "Wrong number of vertices after subdivision with sqrt3";
    EXPECT_EQ(162u, mesh_.n_faces() )    << "Wrong number of faces after subdivision with sqrt3";
  }
}


TEST_F(OpenMeshSubdividerUniform_Triangle, Subdivider_Loop) {
    mesh_.clear();

    // Add some vertices
    Mesh::VertexHandle vhandle[9];

    vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
    vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
    vhandle[2] = mesh_.add_vertex(Mesh::Point(0, 2, 0));
    vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
    vhandle[4] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
    vhandle[5] = mesh_.add_vertex(Mesh::Point(1, 2, 0));
    vhandle[6] = mesh_.add_vertex(Mesh::Point(2, 0, 0));
    vhandle[7] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
    vhandle[8] = mesh_.add_vertex(Mesh::Point(2, 2, 0));

    // Add eight faces
    std::vector<Mesh::VertexHandle> face_vhandles;

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[3]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[6]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[8]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[8]);

    mesh_.add_face(face_vhandles);

    // Test setup:
    //  6 === 7 === 8
    //  |   / |   / |
    //  |  /  |  /  |
    //  | /   | /   |
    //  3 === 4 === 5
    //  |   / | \   |
    //  |  /  |  \  |
    //  | /   |   \ |
    //  0 === 1 === 2

    // Initialize subdivider
    OpenMesh::Subdivider::Uniform::LoopT<Mesh> loop;

    // Check setup
    EXPECT_EQ(9u, mesh_.n_vertices() ) << "Wrong number of vertices";
    EXPECT_EQ(8u, mesh_.n_faces() )    << "Wrong number of faces";

    // Execute 3 subdivision steps
    loop.attach(mesh_);
    loop( 3 );
    loop.detach();

    // Check setup
    EXPECT_EQ(289u, mesh_.n_vertices() ) << "Wrong number of vertices after subdivision with loop";
    EXPECT_EQ(512u, mesh_.n_faces() )    << "Wrong number of faces after subdivision with loop";
}



TEST_F(OpenMeshSubdividerUniform_Triangle, Subdivider_Loop_delete_vertex) {

  for (bool collect_garbage : { false, true })
  {
    mesh_.clear();

    // Request status flags to use delete and garbage collection
    mesh_.request_vertex_status();
    mesh_.request_halfedge_status();
    mesh_.request_edge_status();
    mesh_.request_face_status();

    // Add some vertices
    Mesh::VertexHandle vhandle[9];

    vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
    vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
    vhandle[2] = mesh_.add_vertex(Mesh::Point(0, 2, 0));
    vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
    vhandle[4] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
    vhandle[5] = mesh_.add_vertex(Mesh::Point(1, 2, 0));
    vhandle[6] = mesh_.add_vertex(Mesh::Point(2, 0, 0));
    vhandle[7] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
    vhandle[8] = mesh_.add_vertex(Mesh::Point(2, 2, 0));

    // Add eight faces
    std::vector<Mesh::VertexHandle> face_vhandles;

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[3]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[6]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[8]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[8]);

    mesh_.add_face(face_vhandles);

    // Test setup:
    //  6 === 7 === 8
    //  |   / |   / |
    //  |  /  |  /  |
    //  | /   | /   |
    //  3 === 4 === 5
    //  |   / | \   |
    //  |  /  |  \  |
    //  | /   |   \ |
    //  0 === 1 === 2

    // Delete one vertex
    mesh_.delete_vertex(vhandle[1]);
    if (collect_garbage)
    {
      mesh_.garbage_collection();
      // Check setup
      EXPECT_EQ(8u, mesh_.n_vertices() ) << "Wrong number of vertices";
      EXPECT_EQ(6u, mesh_.n_faces() )    << "Wrong number of faces";
    }
    else
    {
      // Check setup
      EXPECT_EQ(9u, mesh_.n_vertices() ) << "Wrong number of vertices";
      EXPECT_EQ(8u, mesh_.n_faces() )    << "Wrong number of faces";
    }

    // Initialize subdivider
    OpenMesh::Subdivider::Uniform::LoopT<Mesh> loop;


    // Execute 3 subdivision steps
    loop.attach(mesh_);
    loop( 3 );
    loop.detach();

    if (!collect_garbage)
      mesh_.garbage_collection(); // if we did not collect garbage before, do so now

    // Check setup
    EXPECT_EQ(225u, mesh_.n_vertices() ) << "Wrong number of vertices after subdivision with loop";
    EXPECT_EQ(384u, mesh_.n_faces() )    << "Wrong number of faces after subdivision with loop";

  }
}



/*
 * ====================================================================
 * Define tests below
 * ====================================================================
 */

/*
 */
TEST_F(OpenMeshSubdividerUniform_Poly, Subdivider_CatmullClark) {
    mesh_.clear();

    // Add some vertices
    Mesh::VertexHandle vhandle[9];

    vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
    vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
    vhandle[2] = mesh_.add_vertex(Mesh::Point(0, 2, 0));
    vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
    vhandle[4] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
    vhandle[5] = mesh_.add_vertex(Mesh::Point(1, 2, 0));
    vhandle[6] = mesh_.add_vertex(Mesh::Point(2, 0, 0));
    vhandle[7] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
    vhandle[8] = mesh_.add_vertex(Mesh::Point(2, 2, 0));

    // Add four faces
    std::vector<Mesh::VertexHandle> face_vhandles;

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[3]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[8]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[6]);

    mesh_.add_face(face_vhandles);

    // Test setup:
    //  6 === 7 === 8
    //  |     |     |
    //  |     |     |
    //  |     |     |
    //  3 === 4 === 5
    //  |     |     |
    //  |     |     |
    //  |     |     |
    //  0 === 1 === 2

    // Initialize subdivider
    OpenMesh::Subdivider::Uniform::CatmullClarkT<PolyMesh> catmull;

    // Check setup
    EXPECT_EQ(9u, mesh_.n_vertices() ) << "Wrong number of vertices";
    EXPECT_EQ(4u, mesh_.n_faces() )    << "Wrong number of faces";

    // Execute 3 subdivision steps
    catmull.attach(mesh_);
    catmull( 3 );
    catmull.detach();

    EXPECT_EQ(289u, mesh_.n_vertices() ) << "Wrong number of vertices after subdivision with catmull clark";
    EXPECT_EQ(256u, mesh_.n_faces() )    << "Wrong number of faces after subdivision with catmull clark";
}


TEST_F(OpenMeshSubdividerUniform_Poly, Subdivider_CatmullClark_delete_vertex) {

  for (bool collect_garbage : { false, true })
  {
    mesh_.clear();

    // Request status flags to use delete and garbage collection
    mesh_.request_vertex_status();
    mesh_.request_halfedge_status();
    mesh_.request_edge_status();
    mesh_.request_face_status();

    // Add some vertices
    Mesh::VertexHandle vhandle[9];

    vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
    vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
    vhandle[2] = mesh_.add_vertex(Mesh::Point(0, 2, 0));
    vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
    vhandle[4] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
    vhandle[5] = mesh_.add_vertex(Mesh::Point(1, 2, 0));
    vhandle[6] = mesh_.add_vertex(Mesh::Point(2, 0, 0));
    vhandle[7] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
    vhandle[8] = mesh_.add_vertex(Mesh::Point(2, 2, 0));

    // Add four faces
    std::vector<Mesh::VertexHandle> face_vhandles;

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[3]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[8]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[6]);

    mesh_.add_face(face_vhandles);

    // Test setup:
    //  6 === 7 === 8
    //  |     |     |
    //  |     |     |
    //  |     |     |
    //  3 === 4 === 5
    //  |     |     |
    //  |     |     |
    //  |     |     |
    //  0 === 1 === 2


    mesh_.delete_vertex(vhandle[1]);
    // Check setup
    if (collect_garbage)
    {
      mesh_.garbage_collection();
      EXPECT_EQ(6u, mesh_.n_vertices() ) << "Wrong number of vertices";
      EXPECT_EQ(2u, mesh_.n_faces() )    << "Wrong number of faces";
    }
    else
    {
      EXPECT_EQ(9u, mesh_.n_vertices() ) << "Wrong number of vertices";
      EXPECT_EQ(4u, mesh_.n_faces() )    << "Wrong number of faces";
    }

    // Initialize subdivider
    OpenMesh::Subdivider::Uniform::CatmullClarkT<PolyMesh> catmull;

    // Execute 3 subdivision steps
    catmull.attach(mesh_);
    catmull( 3 );
    catmull.detach();

    if (!collect_garbage)
      mesh_.garbage_collection(); // if we did not collect garbage before, do so now

    EXPECT_EQ(153u, mesh_.n_vertices() ) << "Wrong number of vertices after subdivision with catmull clark";
    EXPECT_EQ(128u, mesh_.n_faces() )    << "Wrong number of faces after subdivision with catmull clark";
  }
}

/* Adds a cube to a polymesh
 */
TEST_F(OpenMeshSubdividerUniform_Poly, Midpoint) {
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
    EXPECT_EQ(12u, mesh_.n_edges())     << "Wrong number of Edges";
    EXPECT_EQ(24u, mesh_.n_halfedges()) << "Wrong number of HalfEdges";
    EXPECT_EQ(8u, mesh_.n_vertices())   << "Wrong number of vertices";
    EXPECT_EQ(6u, mesh_.n_faces())      << "Wrong number of faces";

    // Initialize subdivider
    OpenMesh::Subdivider::Uniform::MidpointT<PolyMesh> midpoint;

    // Execute 2 subdivision steps
    midpoint.attach(mesh_);
    midpoint(2);
    midpoint.detach();

    // Check Result
    EXPECT_EQ(48u, mesh_.n_edges())     << "Wrong number of Edges";
    EXPECT_EQ(96u, mesh_.n_halfedges()) << "Wrong number of HalfEdges";
    EXPECT_EQ(24u, mesh_.n_vertices())  << "Wrong number of vertices";
    EXPECT_EQ(26u, mesh_.n_faces())     << "Wrong number of faces";
}


TEST_F(OpenMeshSubdividerUniform_Poly, Midpoint_delete_vertex) {

  for (bool collect_garbage : { false, true })
  {
    mesh_.clear();

    // Request status flags to use delete and garbage collection
    mesh_.request_vertex_status();
    mesh_.request_halfedge_status();
    mesh_.request_edge_status();
    mesh_.request_face_status();

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

    mesh_.delete_vertex(vhandle[1]);
    // Check setup
    if (collect_garbage)
    {
      mesh_.garbage_collection();
      EXPECT_EQ(9u, mesh_.n_edges())      << "Wrong number of Edges";
      EXPECT_EQ(18u, mesh_.n_halfedges()) << "Wrong number of HalfEdges";
      EXPECT_EQ(7u, mesh_.n_vertices())   << "Wrong number of vertices";
      EXPECT_EQ(3u, mesh_.n_faces())      << "Wrong number of faces";
    }
    else
    {
      EXPECT_EQ(12u, mesh_.n_edges())     << "Wrong number of Edges";
      EXPECT_EQ(24u, mesh_.n_halfedges()) << "Wrong number of HalfEdges";
      EXPECT_EQ(8u, mesh_.n_vertices())   << "Wrong number of vertices";
      EXPECT_EQ(6u, mesh_.n_faces())      << "Wrong number of faces";
    }

    // Check setup

    // Initialize subdivider
    OpenMesh::Subdivider::Uniform::MidpointT<PolyMesh> midpoint;

    // Execute 2 subdivision steps
    midpoint.attach(mesh_);
    midpoint(2);
    midpoint.detach();

    if (!collect_garbage)
      mesh_.garbage_collection(); // if we did not collect garbage before, do so now

    // Check Result
    EXPECT_EQ(15u, mesh_.n_edges())     << "Wrong number of Edges";
    EXPECT_EQ(30u, mesh_.n_halfedges()) << "Wrong number of HalfEdges";
    EXPECT_EQ(12u, mesh_.n_vertices())  << "Wrong number of vertices";
    EXPECT_EQ(4u, mesh_.n_faces())     << "Wrong number of faces";
  }
}



TEST_F(OpenMeshSubdividerUniform_Triangle, Modified_Butterfly) {
    mesh_.clear();

    // Add some vertices
    Mesh::VertexHandle vhandle[9];

    vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
    vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
    vhandle[2] = mesh_.add_vertex(Mesh::Point(0, 2, 0));
    vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
    vhandle[4] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
    vhandle[5] = mesh_.add_vertex(Mesh::Point(1, 2, 0));
    vhandle[6] = mesh_.add_vertex(Mesh::Point(2, 0, 0));
    vhandle[7] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
    vhandle[8] = mesh_.add_vertex(Mesh::Point(2, 2, 0));

    // Add eight faces
    std::vector<Mesh::VertexHandle> face_vhandles;

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[3]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[6]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[8]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[8]);

    mesh_.add_face(face_vhandles);

    // Test setup:
    //  6 === 7 === 8
    //  |   / |   / |
    //  |  /  |  /  |
    //  | /   | /   |
    //  3 === 4 === 5
    //  |   / | \   |
    //  |  /  |  \  |
    //  | /   |   \ |
    //  0 === 1 === 2

    // Initialize subdivider
    OpenMesh::Subdivider::Uniform::ModifiedButterflyT<Mesh> butter;

    // Check setup
    EXPECT_EQ(9u, mesh_.n_vertices() ) << "Wrong number of vertices";
    EXPECT_EQ(8u, mesh_.n_faces() )    << "Wrong number of faces";

    // Execute 3 subdivision steps
    butter.attach(mesh_);
    butter( 3 );
    butter.detach();

    // Check setup
    EXPECT_EQ(289u, mesh_.n_vertices() ) << "Wrong number of vertices after subdivision with loop";
    EXPECT_EQ(512u, mesh_.n_faces() )    << "Wrong number of faces after subdivision with loop";
}



TEST_F(OpenMeshSubdividerUniform_Triangle, Modified_Butterfly_delete_vertex) {

  for (bool collect_garbage : { false, true })
  {
    mesh_.clear();

    // Request status flags to use delete and garbage collection
    mesh_.request_vertex_status();
    mesh_.request_halfedge_status();
    mesh_.request_edge_status();
    mesh_.request_face_status();

    // Add some vertices
    Mesh::VertexHandle vhandle[9];

    vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
    vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
    vhandle[2] = mesh_.add_vertex(Mesh::Point(0, 2, 0));
    vhandle[3] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
    vhandle[4] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
    vhandle[5] = mesh_.add_vertex(Mesh::Point(1, 2, 0));
    vhandle[6] = mesh_.add_vertex(Mesh::Point(2, 0, 0));
    vhandle[7] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
    vhandle[8] = mesh_.add_vertex(Mesh::Point(2, 2, 0));

    // Add eight faces
    std::vector<Mesh::VertexHandle> face_vhandles;

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[3]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[4]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[6]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[8]);
    face_vhandles.push_back(vhandle[7]);

    mesh_.add_face(face_vhandles);
    face_vhandles.clear();

    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[8]);

    mesh_.add_face(face_vhandles);

    // Test setup:
    //  6 === 7 === 8
    //  |   / |   / |
    //  |  /  |  /  |
    //  | /   | /   |
    //  3 === 4 === 5
    //  |   / | \   |
    //  |  /  |  \  |
    //  | /   |   \ |
    //  0 === 1 === 2


    // Delete one vertex
    mesh_.delete_vertex(vhandle[1]);
    // Check setup
    if (collect_garbage)
    {
      mesh_.garbage_collection();
      EXPECT_EQ(8u, mesh_.n_vertices() ) << "Wrong number of vertices";
      EXPECT_EQ(6u, mesh_.n_faces() )    << "Wrong number of faces";
    }
    else
    {
      EXPECT_EQ(9u, mesh_.n_vertices() ) << "Wrong number of vertices";
      EXPECT_EQ(8u, mesh_.n_faces() )    << "Wrong number of faces";
    }

    // Initialize subdivider
    OpenMesh::Subdivider::Uniform::ModifiedButterflyT<Mesh> butter;


    // Execute 3 subdivision steps
    butter.attach(mesh_);
    butter( 3 );
    butter.detach();

    if (!collect_garbage)
      mesh_.garbage_collection(); // if we did not collect garbage before, do so now

    // Check setup
    EXPECT_EQ(225u, mesh_.n_vertices() ) << "Wrong number of vertices after subdivision with butter";
    EXPECT_EQ(384u, mesh_.n_faces() )    << "Wrong number of faces after subdivision with butter";
  }
}




}

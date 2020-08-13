
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <OpenMesh/Tools/Subdivider/Adaptive/Composite/CompositeT.hh>
#include <OpenMesh/Tools/Subdivider/Adaptive/Composite/RulesT.hh>

namespace {

class OpenMeshSubdividerAdaptive_Poly : public OpenMeshBasePoly {

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

class OpenMeshSubdividerAdaptive_Triangle : public OpenMeshBase {

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

struct MeshTraits : public OpenMesh::Subdivider::Adaptive::CompositeTraits {
  typedef OpenMesh::Vec3f Point;
  typedef OpenMesh::Vec3f Normal;

  VertexAttributes(OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal);
  EdgeAttributes(OpenMesh::Attributes::Status);
  FaceAttributes(OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal);
};

typedef OpenMesh::TriMesh_ArrayKernelT<MeshTraits> MyMesh;
typedef MyMesh::VertexHandle VHandle;
typedef MyMesh::FaceHandle FHandle;

TEST_F(OpenMeshSubdividerAdaptive_Triangle, AdaptiveCompositeRefineVertex) {

  MyMesh mesh;
  mesh.request_vertex_status();
  mesh.request_edge_status();
  mesh.request_face_status();
  mesh.request_vertex_normals();
  mesh.request_face_normals();

  // Add some vertices
  VHandle vhandle[9];

  vhandle[0] = mesh.add_vertex(MyMesh::Point(0, 0, 0));
  vhandle[1] = mesh.add_vertex(MyMesh::Point(0, 1, 0));
  vhandle[2] = mesh.add_vertex(MyMesh::Point(0, 2, 0));
  vhandle[3] = mesh.add_vertex(MyMesh::Point(1, 0, 0));
  vhandle[4] = mesh.add_vertex(MyMesh::Point(1, 1, 0));
  vhandle[5] = mesh.add_vertex(MyMesh::Point(1, 2, 0));
  vhandle[6] = mesh.add_vertex(MyMesh::Point(2, 0, 0));
  vhandle[7] = mesh.add_vertex(MyMesh::Point(2, 1, 0));
  vhandle[8] = mesh.add_vertex(MyMesh::Point(2, 2, 0));

  // Add eight faces
  std::vector<VHandle> face_vhandles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[3]);

  mesh.add_face(face_vhandles);
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[4]);

  mesh.add_face(face_vhandles);
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[4]);

  mesh.add_face(face_vhandles);
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[4]);

  mesh.add_face(face_vhandles);
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[6]);

  mesh.add_face(face_vhandles);
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[7]);

  mesh.add_face(face_vhandles);
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[8]);
  face_vhandles.push_back(vhandle[7]);

  mesh.add_face(face_vhandles);
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[8]);

  mesh.add_face(face_vhandles);

  //// Test setup:
  ////  6 === 7 === 8
  ////  |   / |   / |
  ////  |  /  |  /  |
  ////  | /   | /   |
  ////  3 === 4 === 5
  ////  |   / | \   |
  ////  |  /  |  \  |
  ////  | /   |   \ |
  ////  0 === 1 === 2

  // Initialize subdivider
  OpenMesh::Subdivider::Adaptive::CompositeT<MyMesh> subdivider(mesh);

  subdivider.add<OpenMesh::Subdivider::Adaptive::Tvv3<MyMesh> >();
  subdivider.add<OpenMesh::Subdivider::Adaptive::VF<MyMesh> >();
  subdivider.add<OpenMesh::Subdivider::Adaptive::FF<MyMesh> >();
  subdivider.add<OpenMesh::Subdivider::Adaptive::FVc<MyMesh> >();

  subdivider.initialize();

  // Check setup
  EXPECT_EQ(9u, mesh.n_vertices() ) << "Wrong number of vertices";
  EXPECT_EQ(8u, mesh.n_faces() )    << "Wrong number of faces";

  // execute adaptive composite subdivision
  subdivider.refine(vhandle[4]);


  // Check setup
  EXPECT_EQ(17u, mesh.n_vertices() ) << "Wrong number of vertices after subdivision with sqrt3";
  EXPECT_EQ(24u, mesh.n_faces() )    << "Wrong number of faces after subdivision with sqrt3";

}

TEST_F(OpenMeshSubdividerAdaptive_Triangle, AdaptiveCompositeRefineFace) {

  MyMesh mesh;
  mesh.request_vertex_status();
  mesh.request_edge_status();
  mesh.request_face_status();
  mesh.request_vertex_normals();
  mesh.request_face_normals();

  // Add some vertices
  VHandle vhandle[9];

  vhandle[0] = mesh.add_vertex(MyMesh::Point(0, 0, 0));
  vhandle[1] = mesh.add_vertex(MyMesh::Point(0, 1, 0));
  vhandle[2] = mesh.add_vertex(MyMesh::Point(0, 2, 0));
  vhandle[3] = mesh.add_vertex(MyMesh::Point(1, 0, 0));
  vhandle[4] = mesh.add_vertex(MyMesh::Point(1, 1, 0));
  vhandle[5] = mesh.add_vertex(MyMesh::Point(1, 2, 0));
  vhandle[6] = mesh.add_vertex(MyMesh::Point(2, 0, 0));
  vhandle[7] = mesh.add_vertex(MyMesh::Point(2, 1, 0));
  vhandle[8] = mesh.add_vertex(MyMesh::Point(2, 2, 0));

  // Add eight faces
  std::vector<VHandle> face_vhandles;
  std::vector<FHandle> face_handles;

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[3]);

  face_handles.push_back(mesh.add_face(face_vhandles));
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[4]);

  face_handles.push_back(mesh.add_face(face_vhandles));
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[4]);

  face_handles.push_back(mesh.add_face(face_vhandles));
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[4]);

  face_handles.push_back(mesh.add_face(face_vhandles));
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[6]);

  face_handles.push_back(mesh.add_face(face_vhandles));
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[7]);

  face_handles.push_back(mesh.add_face(face_vhandles));
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[8]);
  face_vhandles.push_back(vhandle[7]);

  face_handles.push_back(mesh.add_face(face_vhandles));
  face_vhandles.clear();

  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[8]);

  face_handles.push_back(mesh.add_face(face_vhandles));

  //// Test setup:
  ////  6 === 7 === 8
  ////  |   / |   / |
  ////  |  /  |  /  |
  ////  | /   | /   |
  ////  3 === 4 === 5
  ////  |   / | \   |
  ////  |  /  |  \  |
  ////  | /   |   \ |
  ////  0 === 1 === 2

  // Initialize subdivider
  OpenMesh::Subdivider::Adaptive::CompositeT<MyMesh> subdivider(mesh);

  subdivider.add<OpenMesh::Subdivider::Adaptive::Tvv3<MyMesh> >();
  subdivider.add<OpenMesh::Subdivider::Adaptive::VF<MyMesh> >();
  subdivider.add<OpenMesh::Subdivider::Adaptive::FF<MyMesh> >();
  subdivider.add<OpenMesh::Subdivider::Adaptive::FVc<MyMesh> >();

  subdivider.initialize();

  // Check setup
  EXPECT_EQ(9u, mesh.n_vertices() ) << "Wrong number of vertices";
  EXPECT_EQ(8u, mesh.n_faces() )    << "Wrong number of faces";

  // execute adaptive composite subdivision
  std::vector<FHandle>::iterator it, end;
  it = face_handles.begin();
  end = face_handles.end();
  for (; it != end; ++it)
    subdivider.refine(*it);


  // Check setup
  EXPECT_EQ(245u, mesh.n_vertices() ) << "Wrong number of vertices after subdivision with sqrt3";
  EXPECT_EQ(458u, mesh.n_faces() )    << "Wrong number of faces after subdivision with sqrt3";

}
}

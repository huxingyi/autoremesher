#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <OpenMesh/Core/Mesh/SmartHandles.hh>

#include <iostream>
#include <chrono>

namespace {

class OpenMeshSmartHandles : public OpenMeshBase {

protected:

  // This function is called before each test is run
  virtual void SetUp() {

    mesh_.clear();

    // Add some vertices
    Mesh::VertexHandle vhandle[8];
    vhandle[0] = mesh_.add_vertex(Mesh::Point(-1, -1,  1));
    vhandle[1] = mesh_.add_vertex(Mesh::Point( 1, -1,  1));
    vhandle[2] = mesh_.add_vertex(Mesh::Point( 1,  1,  1));
    vhandle[3] = mesh_.add_vertex(Mesh::Point(-1,  1,  1));
    vhandle[4] = mesh_.add_vertex(Mesh::Point(-1, -1, -1));
    vhandle[5] = mesh_.add_vertex(Mesh::Point( 1, -1, -1));
    vhandle[6] = mesh_.add_vertex(Mesh::Point( 1,  1, -1));
    vhandle[7] = mesh_.add_vertex(Mesh::Point(-1,  1, -1));

    // Add six faces to form a cube
    std::vector<Mesh::VertexHandle> face_vhandles;

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[3]);
    mesh_.add_face(face_vhandles);

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[3]);
    mesh_.add_face(face_vhandles);

    //=======================

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[6]);
    face_vhandles.push_back(vhandle[5]);
    mesh_.add_face(face_vhandles);

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[4]);
    mesh_.add_face(face_vhandles);

    //=======================

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[4]);
    mesh_.add_face(face_vhandles);

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[5]);
    mesh_.add_face(face_vhandles);

    //=======================

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[5]);
    mesh_.add_face(face_vhandles);

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[6]);
    mesh_.add_face(face_vhandles);


    //=======================

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[6]);
    mesh_.add_face(face_vhandles);

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[6]);
    face_vhandles.push_back(vhandle[7]);
    mesh_.add_face(face_vhandles);

    //=======================

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[7]);
    mesh_.add_face(face_vhandles);

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[0]);
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
    EXPECT_EQ(18u, mesh_.n_edges() )     << "Wrong number of Edges";
    EXPECT_EQ(36u, mesh_.n_halfedges() ) << "Wrong number of HalfEdges";
    EXPECT_EQ(8u, mesh_.n_vertices() )   << "Wrong number of vertices";
    EXPECT_EQ(12u, mesh_.n_faces() )     << "Wrong number of faces";
  }

  // This function is called after all tests are through
  virtual void TearDown() {

    // Do some final stuff with the member data here...

    mesh_.clear();
  }

  // Member already defined in OpenMeshBase
  //Mesh mesh_;
};

/*
 * ====================================================================
 * Define tests below
 * ====================================================================
 */



/* Test if navigation operations on smart handles yield the expected element
 */
TEST_F(OpenMeshSmartHandles, SimpleNavigation)
{
  for (auto vh : mesh_.vertices())
  {
    EXPECT_EQ(mesh_.halfedge_handle(vh), vh.halfedge()) << "outgoing halfedge of vertex does not match";
  }

  for (auto heh : mesh_.halfedges())
  {
    EXPECT_EQ(mesh_.next_halfedge_handle(heh),     heh.next()) << "next halfedge of halfedge does not match";
    EXPECT_EQ(mesh_.prev_halfedge_handle(heh),     heh.prev()) << "prevt halfedge of halfedge does not match";
    EXPECT_EQ(mesh_.opposite_halfedge_handle(heh), heh.opp())  << "opposite halfedge of halfedge does not match";
    EXPECT_EQ(mesh_.to_vertex_handle(heh),         heh.to())   << "to vertex handle of halfedge does not match";
    EXPECT_EQ(mesh_.from_vertex_handle(heh),       heh.from()) << "from vertex handle of halfedge does not match";
    EXPECT_EQ(mesh_.face_handle(heh),              heh.face()) << "face handle of halfedge does not match";
  }

  for (auto eh : mesh_.edges())
  {
    EXPECT_EQ(mesh_.halfedge_handle(eh, 0), eh.h0()) << "halfedge 0 of edge does not match";
    EXPECT_EQ(mesh_.halfedge_handle(eh, 1), eh.h1()) << "halfedge 1 of edge does not match";
    EXPECT_EQ(mesh_.from_vertex_handle(mesh_.halfedge_handle(eh, 0)), eh.v0()) << "first vertex of edge does not match";
    EXPECT_EQ(mesh_.to_vertex_handle  (mesh_.halfedge_handle(eh, 0)), eh.v1()) << "second vertex of edge does not match";
  }

  for (auto fh : mesh_.faces())
  {
    EXPECT_EQ(mesh_.halfedge_handle(fh), fh.halfedge()) << "halfedge of face does not match";
  }
}


/* Test if ranges yield the same elements when using smart handles
 */
TEST_F(OpenMeshSmartHandles, SimpleRanges)
{
  for (auto vh : mesh_.vertices())
  {
    {
      std::vector<OpenMesh::VertexHandle> handles0;
      std::vector<OpenMesh::VertexHandle> handles1;
      for (auto h : mesh_.vv_range(vh))
        handles0.push_back(h);
      for (auto h : vh.vertices())
        handles1.push_back(h);
      EXPECT_EQ(handles0, handles1) << "vertex range of vertex does not match";
    }
    {
      std::vector<OpenMesh::HalfedgeHandle> handles0;
      std::vector<OpenMesh::HalfedgeHandle> handles1;
      for (auto h : mesh_.voh_range(vh))
        handles0.push_back(h);
      for (auto h : vh.outgoing_halfedges())
        handles1.push_back(h);
      EXPECT_EQ(handles0, handles1) << "outgoing halfedge range of vertex does not match";
    }
    {
      std::vector<OpenMesh::HalfedgeHandle> handles0;
      std::vector<OpenMesh::HalfedgeHandle> handles1;
      for (auto h : mesh_.vih_range(vh))
        handles0.push_back(h);
      for (auto h : vh.incoming_halfedges())
        handles1.push_back(h);
      EXPECT_EQ(handles0, handles1) << "incoming halfedge range of vertex does not match";
    }
    {
      std::vector<OpenMesh::EdgeHandle> handles0;
      std::vector<OpenMesh::EdgeHandle> handles1;
      for (auto h : mesh_.ve_range(vh))
        handles0.push_back(h);
      for (auto h : vh.edges())
        handles1.push_back(h);
      EXPECT_EQ(handles0, handles1) << "edge range of vertex does not match";
    }
    {
      std::vector<OpenMesh::FaceHandle> handles0;
      std::vector<OpenMesh::FaceHandle> handles1;
      for (auto h : mesh_.vf_range(vh))
        handles0.push_back(h);
      for (auto h : vh.faces())
        handles1.push_back(h);
      EXPECT_EQ(handles0, handles1) << "face range of vertex does not match";
    }
  }

  for (auto fh : mesh_.faces())
  {
    {
      std::vector<OpenMesh::VertexHandle> handles0;
      std::vector<OpenMesh::VertexHandle> handles1;
      for (auto h : mesh_.fv_range(fh))
        handles0.push_back(h);
      for (auto h : fh.vertices())
        handles1.push_back(h);
      EXPECT_EQ(handles0, handles1) << "vertex range of face does not match";
    }
    {
      std::vector<OpenMesh::HalfedgeHandle> handles0;
      std::vector<OpenMesh::HalfedgeHandle> handles1;
      for (auto h : mesh_.fh_range(fh))
        handles0.push_back(h);
      for (auto h : fh.halfedges())
        handles1.push_back(h);
      EXPECT_EQ(handles0, handles1) << "halfedge range of face does not match";
    }
    {
      std::vector<OpenMesh::EdgeHandle> handles0;
      std::vector<OpenMesh::EdgeHandle> handles1;
      for (auto h : mesh_.fe_range(fh))
        handles0.push_back(h);
      for (auto h : fh.edges())
        handles1.push_back(h);
      EXPECT_EQ(handles0, handles1) << "edge range of face does not match";
    }
    {
      std::vector<OpenMesh::FaceHandle> handles0;
      std::vector<OpenMesh::FaceHandle> handles1;
      for (auto h : mesh_.ff_range(fh))
        handles0.push_back(h);
      for (auto h : fh.faces())
        handles1.push_back(h);
      EXPECT_EQ(handles0, handles1) << "face range of face does not match";
    }
  }
}

/* Test if ranges yield the same elements when using smart handles
 */
TEST_F(OpenMeshSmartHandles, RangesOfRanges)
{
  for (auto vh : mesh_.vertices())
  {
    {
      std::vector<OpenMesh::VertexHandle> handles0;
      std::vector<OpenMesh::VertexHandle> handles1;
      for (auto h : mesh_.vv_range(vh))
        for (auto h2 : mesh_.vv_range(h))
          handles0.push_back(h2);
      for (auto h : vh.vertices())
        for (auto h2 : h.vertices())
          handles1.push_back(h2);
      EXPECT_EQ(handles0, handles1) << "vertex range of vertex range does not match";
    }
    {
      std::vector<OpenMesh::HalfedgeHandle> handles0;
      std::vector<OpenMesh::HalfedgeHandle> handles1;
      for (auto h : mesh_.vv_range(vh))
        for (auto h2 : mesh_.voh_range(h))
          handles0.push_back(h2);
      for (auto h : vh.vertices())
        for (auto h2 : h.outgoing_halfedges())
          handles1.push_back(h2);
      EXPECT_EQ(handles0, handles1) << "outgoing halfedge range of vertex range does not match";
    }
    {
      std::vector<OpenMesh::HalfedgeHandle> handles0;
      std::vector<OpenMesh::HalfedgeHandle> handles1;
      for (auto h : mesh_.vv_range(vh))
        for (auto h2 : mesh_.vih_range(h))
          handles0.push_back(h2);
      for (auto h : vh.vertices())
        for (auto h2 : h.incoming_halfedges())
          handles1.push_back(h2);
      EXPECT_EQ(handles0, handles1) << "incoming halfedge range of vertex range does not match";
    }
    {
      std::vector<OpenMesh::EdgeHandle> handles0;
      std::vector<OpenMesh::EdgeHandle> handles1;
      for (auto h : mesh_.vv_range(vh))
        for (auto h2 : mesh_.ve_range(h))
          handles0.push_back(h2);
      for (auto h : vh.vertices())
        for (auto h2 : h.edges())
          handles1.push_back(h2);
      EXPECT_EQ(handles0, handles1) << "edge range of vertex range does not match";
    }
    {
      std::vector<OpenMesh::FaceHandle> handles0;
      std::vector<OpenMesh::FaceHandle> handles1;
      for (auto h : mesh_.vv_range(vh))
        for (auto h2 : mesh_.vf_range(h))
          handles0.push_back(h2);
      for (auto h : vh.vertices())
        for (auto h2 : h.faces())
          handles1.push_back(h2);
      EXPECT_EQ(handles0, handles1) << "face range of vertex range does not match";
    }
    {
      std::vector<OpenMesh::VertexHandle> handles0;
      std::vector<OpenMesh::VertexHandle> handles1;
      for (auto h : mesh_.vf_range(vh))
        for (auto h2 : mesh_.fv_range(h))
          handles0.push_back(h2);
      for (auto h : vh.faces())
        for (auto h2 : h.vertices())
          handles1.push_back(h2);
      EXPECT_EQ(handles0, handles1) << "vertex range of face range does not match";
    }
    {
      std::vector<OpenMesh::HalfedgeHandle> handles0;
      std::vector<OpenMesh::HalfedgeHandle> handles1;
      for (auto h : mesh_.vf_range(vh))
        for (auto h2 : mesh_.fh_range(h))
          handles0.push_back(h2);
      for (auto h : vh.faces())
        for (auto h2 : h.halfedges())
          handles1.push_back(h2);
      EXPECT_EQ(handles0, handles1) << "vertex range of face range does not match";
    }
    {
      std::vector<OpenMesh::FaceHandle> handles0;
      std::vector<OpenMesh::FaceHandle> handles1;
      for (auto h : mesh_.vf_range(vh))
        for (auto h2 : mesh_.ff_range(h))
          handles0.push_back(h2);
      for (auto h : vh.faces())
        for (auto h2 : h.faces())
          handles1.push_back(h2);
      EXPECT_EQ(handles0, handles1) << "vertex range of face range does not match";
    }
  }
}


/* Test a chain of navigation on a cube
 */
TEST_F(OpenMeshSmartHandles, ComplicatedNavigtaion)
{
  for (auto vh : mesh_.vertices())
  {
    EXPECT_EQ(mesh_.next_halfedge_handle(
              mesh_.opposite_halfedge_handle(
              mesh_.halfedge_handle(vh))),
              vh.out().opp().next());
    EXPECT_EQ(mesh_.prev_halfedge_handle(
              mesh_.prev_halfedge_handle(
              mesh_.opposite_halfedge_handle(
              mesh_.next_halfedge_handle(
              mesh_.next_halfedge_handle(
              mesh_.halfedge_handle(vh)))))),
              vh.out().next().next().opp().prev().prev());
    EXPECT_EQ(mesh_.face_handle(
              mesh_.opposite_halfedge_handle(
              mesh_.halfedge_handle(
              mesh_.face_handle(
              mesh_.opposite_halfedge_handle(
              mesh_.next_halfedge_handle(
              mesh_.halfedge_handle(vh))))))),
              vh.out().next().opp().face().halfedge().opp().face());
  }
}


/* Test performance of smart handles
 */
TEST_F(OpenMeshSmartHandles, Performance)
{
#if NDEBUG
  int n_tests = 10000000;
#else
  int n_tests = 300000;
#endif

  auto t_before_old = std::chrono::high_resolution_clock::now();

  std::vector<OpenMesh::HalfedgeHandle> halfedges0;
  for (int i = 0; i < n_tests; ++i)
  {
    for (auto vh : mesh_.vertices())
    {
      auto heh = mesh_.prev_halfedge_handle(
                 mesh_.prev_halfedge_handle(
                 mesh_.opposite_halfedge_handle(
                 mesh_.next_halfedge_handle(
                 mesh_.next_halfedge_handle(
                 mesh_.halfedge_handle(vh))))));
      if (i == 0)
        halfedges0.push_back(heh);
    }
  }

  auto t_after_old = std::chrono::high_resolution_clock::now();

  std::vector<OpenMesh::HalfedgeHandle> halfedges1;
  for (int i = 0; i < n_tests; ++i)
  {
    for (auto vh : mesh_.vertices())
    {
      auto heh = vh.out().next().next().opp().prev().prev();
      if (i == 0)
        halfedges1.push_back(heh);
    }
  }

  auto t_after_new = std::chrono::high_resolution_clock::now();

  std::cout << "Conventional navigation took " << std::chrono::duration_cast<std::chrono::milliseconds>(t_after_old-t_before_old).count() << "ms" << std::endl;
  std::cout << "SmartHandle  navigation took " << std::chrono::duration_cast<std::chrono::milliseconds>(t_after_new-t_after_old ).count() << "ms" << std::endl;

  EXPECT_EQ(halfedges0, halfedges1) << "halfedges do not match";

}


/* Mix old and new api
 */
TEST_F(OpenMeshSmartHandles, MixOldAndNew)
{
  for (OpenMesh::SmartHalfedgeHandle heh : mesh_.halfedges())
  {
    heh = mesh_.opposite_halfedge_handle(heh);
    EXPECT_TRUE((std::is_same<OpenMesh::SmartEdgeHandle, decltype(OpenMesh::PolyConnectivity::s_edge_handle(heh))>::value));
    EXPECT_TRUE((std::is_same<OpenMesh::SmartEdgeHandle, decltype(mesh_.edge_handle(heh))>::value));
    EXPECT_TRUE((std::is_same<OpenMesh::SmartFaceHandle, decltype(mesh_.face_handle(heh))>::value));
  }
}



/* comparability
 */
TEST_F(OpenMeshSmartHandles, ComparisionBetweenSmartHandleAndNormalHandles)
{
  OpenMesh::VertexHandle vh(0);
  OpenMesh::SmartVertexHandle svh(0, &mesh_);
  EXPECT_EQ(vh, svh) << "Vertex handle and smart vertex handle are different";

  std::vector<OpenMesh::VertexHandle> vertices = mesh_.vertices().to_vector([](OpenMesh::SmartVertexHandle _svh) { return OpenMesh::VertexHandle(_svh); });

  std::replace(vertices.begin(), vertices.end(), OpenMesh::VertexHandle(0), OpenMesh::VertexHandle(1));
  EXPECT_EQ(vertices[0], OpenMesh::VertexHandle(1));

  std::vector<OpenMesh::SmartVertexHandle> smart_vertices = mesh_.vertices().to_vector();

  std::replace(smart_vertices.begin(), smart_vertices.end(), OpenMesh::SmartVertexHandle(0, &mesh_), OpenMesh::SmartVertexHandle(1, &mesh_));
  EXPECT_EQ(smart_vertices[0], OpenMesh::VertexHandle(1));
  EXPECT_EQ(smart_vertices[0], OpenMesh::SmartVertexHandle(1, &mesh_));

  std::replace(vertices.begin(), vertices.end(),  OpenMesh::SmartVertexHandle(1, &mesh_), OpenMesh::SmartVertexHandle(2, &mesh_));
  EXPECT_EQ(vertices[0], OpenMesh::VertexHandle(2));

}

TEST(OpenMeshSmartHandlesNoFixture, AddingFacesPolyMesh)
{
  using MyMesh = OpenMesh::PolyMesh_ArrayKernelT<>;

  MyMesh mesh;

  std::vector<OpenMesh::SmartVertexHandle> vertices;
  for (int i = 0; i < 4; ++i)
    vertices.push_back(mesh.add_vertex(MyMesh::Point()));

  auto fh = mesh.add_face(vertices);

  for (auto heh : fh.halfedges())
  {
    heh = heh.next();
  }
}

TEST(OpenMeshSmartHandlesNoFixture, AddingFacesTriMesh)
{
  using MyMesh = OpenMesh::TriMesh_ArrayKernelT<>;

  MyMesh mesh;

  std::vector<OpenMesh::SmartVertexHandle> vertices;
  for (int i = 0; i < 4; ++i)
    vertices.push_back(mesh.add_vertex(MyMesh::Point()));

  auto fh = mesh.add_face(vertices);

  for (auto heh : fh.halfedges())
  {
    heh = heh.next();
  }
}

TEST(OpenMeshSmartHandlesNoFixture, SplitTriMesh)
{
  using MyMesh = OpenMesh::TriMesh_ArrayKernelT<>;

  MyMesh mesh;

  std::vector<OpenMesh::SmartVertexHandle> vertices;
  for (int i = 0; i < 3; ++i)
    vertices.push_back(mesh.add_vertex(MyMesh::Point()));

  auto fh = mesh.add_face(vertices);

  auto p = (MyMesh::Point());

  OpenMesh::SmartVertexHandle vh = mesh.split(fh, p);
  OpenMesh::SmartEdgeHandle eh = fh.halfedge().edge();
  OpenMesh::SmartVertexHandle vh2 = mesh.split(eh, p);

  EXPECT_NE(vh.idx(), vh2.idx()) << "This was only intended to fix an unused variable warning but cool that it caugth an actual error now";

}





}

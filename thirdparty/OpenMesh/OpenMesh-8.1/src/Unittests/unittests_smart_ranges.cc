#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <OpenMesh/Core/Mesh/SmartHandles.hh>
#include <OpenMesh/Core/Utils/PropertyManager.hh>

#include <iostream>
#include <chrono>

namespace {

class OpenMeshSmartRanges : public OpenMeshBase {

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


template <typename HandleT>
struct F
{
  unsigned int operator()(HandleT ) { return 1; }
};

/* Test if smart ranges work
 */
TEST_F(OpenMeshSmartRanges, Sum)
{
  auto one = [](OpenMesh::VertexHandle ) { return 1u; };
  EXPECT_EQ(mesh_.vertices().sum(one), mesh_.n_vertices());
  EXPECT_EQ(mesh_.vertices().sum(F<OpenMesh::VertexHandle>()), mesh_.n_vertices());
  EXPECT_EQ(mesh_.halfedges().sum(F<OpenMesh::HalfedgeHandle>()), mesh_.n_halfedges());
  EXPECT_EQ(mesh_.edges().sum(F<OpenMesh::EdgeHandle>()), mesh_.n_edges());
  EXPECT_EQ(mesh_.faces().sum(F<OpenMesh::FaceHandle>()), mesh_.n_faces());

  for (auto vh : mesh_.vertices())
    EXPECT_EQ(vh.vertices().sum(F<OpenMesh::VertexHandle>()), mesh_.valence(vh));
  for (auto vh : mesh_.vertices())
    EXPECT_EQ(vh.faces().sum(F<OpenMesh::FaceHandle>()), mesh_.valence(vh));
  for (auto vh : mesh_.vertices())
    EXPECT_EQ(vh.outgoing_halfedges().sum(F<OpenMesh::HalfedgeHandle>()), mesh_.valence(vh));
  for (auto vh : mesh_.vertices())
    EXPECT_EQ(vh.incoming_halfedges().sum(F<OpenMesh::HalfedgeHandle>()), mesh_.valence(vh));

  for (auto fh : mesh_.faces())
    EXPECT_EQ(fh.vertices().sum(F<OpenMesh::VertexHandle>()), mesh_.valence(fh));
  for (auto fh : mesh_.faces())
    EXPECT_EQ(fh.halfedges().sum(F<OpenMesh::HalfedgeHandle>()), mesh_.valence(fh));
  for (auto fh : mesh_.faces())
    EXPECT_EQ(fh.edges().sum(F<OpenMesh::EdgeHandle>()), mesh_.valence(fh));
  for (auto fh : mesh_.faces())
    EXPECT_EQ(fh.faces().sum(F<OpenMesh::FaceHandle>()), 3u);
}


/* Test if Property Manager can be used in smart ranges
 */
TEST_F(OpenMeshSmartRanges, PropertyManagerAsFunctor)
{
  OpenMesh::VProp<Mesh::Point> myPos(mesh_);
  for (auto vh : mesh_.vertices())
    myPos(vh) = mesh_.point(vh);

  Mesh::Point cog(0,0,0);
  for (auto vh : mesh_.vertices())
    cog += mesh_.point(vh);
  cog /= mesh_.n_vertices();

  auto cog2 = mesh_.vertices().avg(myPos);

  EXPECT_LT(norm(cog - cog2), 0.00001) << "Computed center of gravities are significantly different.";
}

/* Test to vector
 */
TEST_F(OpenMeshSmartRanges, ToVector)
{
  OpenMesh::HProp<OpenMesh::Vec2d> uvs(mesh_);

  for (auto heh : mesh_.halfedges())
    uvs(heh) = OpenMesh::Vec2d(heh.idx(), (heh.idx() * 13)%7);

  for (auto fh : mesh_.faces())
  {
    auto tri_uvs = fh.halfedges().to_vector(uvs);
    auto heh_handles = fh.halfedges().to_vector();
    for (auto heh : heh_handles)
      heh.next();
  }

  auto vertex_vec = mesh_.vertices().to_vector();
  for (auto vh : vertex_vec)
    vh.out();
}

/* Test to array
 */
TEST_F(OpenMeshSmartRanges, ToArray)
{
  OpenMesh::HProp<OpenMesh::Vec2d> uvs(mesh_);

  for (auto heh : mesh_.halfedges())
    uvs(heh) = OpenMesh::Vec2d(heh.idx(), (heh.idx() * 13)%7);

  for (auto fh : mesh_.faces())
  {
    fh.halfedges().to_array<3>(uvs);
    fh.halfedges().to_array<3>();
  }
}


/* Test bounding box
 */
TEST_F(OpenMeshSmartRanges, BoundingBox)
{
  // The custom vecs OpenMesh are tested with here do not implement a min or max function.
  // Thus we convert here.
  OpenMesh::VProp<OpenMesh::Vec3f> myPos(mesh_);
  for (auto vh : mesh_.vertices())
    for (size_t i = 0; i < 3; ++i)
      myPos(vh)[i] = mesh_.point(vh)[i];

  auto bb_min = mesh_.vertices().min(myPos);
  auto bb_max = mesh_.vertices().max(myPos);
  mesh_.vertices().minmax(myPos);

  EXPECT_LT(norm(bb_min - OpenMesh::Vec3f(-1,-1,-1)), 0.000001) << "Bounding box minimum seems off";
  EXPECT_LT(norm(bb_max - OpenMesh::Vec3f( 1, 1, 1)), 0.000001) << "Bounding box maximum seems off";


  auto uvs = OpenMesh::makeTemporaryProperty<OpenMesh::HalfedgeHandle, OpenMesh::Vec2d>(mesh_);
  for (auto heh : mesh_.halfedges())
    uvs(heh) = OpenMesh::Vec2d(heh.idx(), (heh.idx() * 13)%7);

  for (auto fh : mesh_.faces())
  {
    fh.halfedges().min(uvs);
    fh.halfedges().max(uvs);
  }
}


/* Test for each
 */
TEST_F(OpenMeshSmartRanges, ForEach)
{
  std::vector<int> vec;
  auto f = [&vec](OpenMesh::VertexHandle vh) { vec.push_back(vh.idx()); };

  mesh_.vertices().for_each(f);

  ASSERT_EQ(vec.size(), mesh_.n_vertices()) << "vec has wrong size";
  for (size_t i = 0; i < vec.size(); ++i)
    EXPECT_EQ(vec[i], i) << "wrong index in vector";
}


/* Test filter
 */
TEST_F(OpenMeshSmartRanges, Filtered)
{
  using VH = OpenMesh::VertexHandle;

  auto is_even            = [](VH vh) { return vh.idx() % 2 == 0; };
  auto is_odd             = [](VH vh) { return vh.idx() % 2 == 1; };
  auto is_divisible_by_3  = [](VH vh) { return vh.idx() % 3 == 0; };
  auto to_id              = [](VH vh) { return vh.idx(); };

  auto even_vertices = mesh_.vertices().filtered(is_even).to_vector(to_id);
  EXPECT_EQ(even_vertices.size(), 4);
  EXPECT_EQ(even_vertices[0], 0);
  EXPECT_EQ(even_vertices[1], 2);
  EXPECT_EQ(even_vertices[2], 4);
  EXPECT_EQ(even_vertices[3], 6);

  auto odd_vertices = mesh_.vertices().filtered(is_odd).to_vector(to_id);
  EXPECT_EQ(odd_vertices.size(), 4);
  EXPECT_EQ(odd_vertices[0], 1);
  EXPECT_EQ(odd_vertices[1], 3);
  EXPECT_EQ(odd_vertices[2], 5);
  EXPECT_EQ(odd_vertices[3], 7);

  auto even_3_vertices = mesh_.vertices().filtered(is_even).filtered(is_divisible_by_3).to_vector(to_id);
  EXPECT_EQ(even_3_vertices.size(), 2);
  EXPECT_EQ(even_3_vertices[0], 0);
  EXPECT_EQ(even_3_vertices[1], 6);

  auto odd_3_vertices = mesh_.vertices().filtered(is_odd).filtered(is_divisible_by_3).to_vector(to_id);
  EXPECT_EQ(odd_3_vertices.size(), 1);
  EXPECT_EQ(odd_3_vertices[0], 3);


  // create a vector of vertices in the order they are visited when iterating over face vertices, but every vertex only once
  std::vector<VH> vertices;
  OpenMesh::VProp<bool> to_be_processed(true, mesh_);
  auto store_vertex = [&](VH vh) { to_be_processed(vh) = false; vertices.push_back(vh); };

  for (auto fh : mesh_.faces())
    fh.vertices().filtered(to_be_processed).for_each(store_vertex);

  EXPECT_EQ(vertices.size(), mesh_.n_vertices()) << " number of visited vertices not correct";
  EXPECT_TRUE(mesh_.vertices().all_of([&](VH vh) { return !to_be_processed(vh); })) << "did not visit all vertices";

}



}

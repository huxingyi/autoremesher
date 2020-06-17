#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <OpenMesh/Core/Utils/PropertyManager.hh>

#include <iostream>

//#define ENABLE_PROPERTY_TIMING_OUTPUT
#ifdef ENABLE_PROPERTY_TIMING_OUTPUT
#define N_VERTICES_TIMING 1000000
#define TIMING_OUTPUT(X) X
#else
#define N_VERTICES_TIMING 10
#define TIMING_OUTPUT(X)
#endif

namespace {

class OpenMeshPropertyManager : public OpenMeshBase {

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
 * General Tests
 * ====================================================================
 */

/*
 * Collapsing a tetrahedron
 */
TEST_F(OpenMeshPropertyManager, set_range_bool) {

  mesh_.clear();

  // Add some vertices
  Mesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0, 0, 1));

  // Add two faces
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

  {
      OpenMesh::PropertyManager<
          OpenMesh::VPropHandleT<bool>> pm_v_bool(mesh_, "pm_v_bool");
      pm_v_bool.set_range(mesh_.vertices_begin(), mesh_.vertices_end(), false);
      for (int i = 0; i < 4; ++i)
          ASSERT_FALSE(pm_v_bool[vhandle[i]]);
      pm_v_bool.set_range(mesh_.vertices_begin(), mesh_.vertices_end(), true);
      for (int i = 0; i < 4; ++i)
          ASSERT_TRUE(pm_v_bool[vhandle[i]]);

      OpenMesh::PropertyManager<
          OpenMesh::EPropHandleT<bool>> pm_e_bool(mesh_, "pm_e_bool");
      pm_e_bool.set_range(mesh_.edges_begin(), mesh_.edges_end(), false);
      for (Mesh::EdgeIter e_it = mesh_.edges_begin(), f_end = mesh_.edges_end();
              e_it != f_end; ++e_it)
          ASSERT_FALSE(pm_e_bool[*e_it]);
      pm_e_bool.set_range(mesh_.edges_begin(), mesh_.edges_end(), true);
      for (Mesh::EdgeIter e_it = mesh_.edges_begin(), f_end = mesh_.edges_end();
              e_it != f_end; ++e_it)
          ASSERT_TRUE(pm_e_bool[*e_it]);

      OpenMesh::PropertyManager<
          OpenMesh::FPropHandleT<bool>> pm_f_bool(mesh_, "pm_f_bool");
      pm_f_bool.set_range(mesh_.faces_begin(), mesh_.faces_end(), false);
      for (Mesh::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end();
              f_it != f_end; ++f_it)
          ASSERT_FALSE(pm_f_bool[*f_it]);
      pm_f_bool.set_range(mesh_.faces_begin(), mesh_.faces_end(), true);
      for (Mesh::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end();
              f_it != f_end; ++f_it)
          ASSERT_TRUE(pm_f_bool[*f_it]);
  }

  /*
   * Same thing again, this time with C++11 ranges.
   */
  {
      OpenMesh::PropertyManager<
          OpenMesh::VPropHandleT<bool>> pm_v_bool(mesh_, "pm_v_bool2");
      pm_v_bool.set_range(mesh_.vertices(), false);
      for (int i = 0; i < 4; ++i)
          ASSERT_FALSE(pm_v_bool[vhandle[i]]);
      pm_v_bool.set_range(mesh_.vertices(), true);
      for (int i = 0; i < 4; ++i)
          ASSERT_TRUE(pm_v_bool[vhandle[i]]);

      OpenMesh::PropertyManager<
          OpenMesh::EPropHandleT<bool>> pm_e_bool(mesh_, "pm_e_bool2");
      pm_e_bool.set_range(mesh_.edges(), false);
      for (Mesh::EdgeIter e_it = mesh_.edges_begin(), f_end = mesh_.edges_end();
              e_it != f_end; ++e_it)
          ASSERT_FALSE(pm_e_bool[*e_it]);
      pm_e_bool.set_range(mesh_.edges(), true);
      for (Mesh::EdgeIter e_it = mesh_.edges_begin(), f_end = mesh_.edges_end();
              e_it != f_end; ++e_it)
          ASSERT_TRUE(pm_e_bool[*e_it]);

      OpenMesh::PropertyManager<
          OpenMesh::FPropHandleT<bool>> pm_f_bool(mesh_, "pm_f_bool2");
      pm_f_bool.set_range(mesh_.faces(), false);
      for (Mesh::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end();
              f_it != f_end; ++f_it)
          ASSERT_FALSE(pm_f_bool[*f_it]);
      pm_f_bool.set_range(mesh_.faces(), true);
      for (Mesh::FaceIter f_it = mesh_.faces_begin(), f_end = mesh_.faces_end();
              f_it != f_end; ++f_it)
          ASSERT_TRUE(pm_f_bool[*f_it]);
  }
}

/*
 * In sequence:
 * - add a persistent property to a mesh
 * - retrieve an existing property of a mesh and modify it
 * - obtain a non-owning property handle
 * - attempt to obtain a non-owning handle to a non-existing property (throws)
 */
TEST_F(OpenMeshPropertyManager, cpp11_persistent_and_non_owning_properties) {
    auto vh = mesh_.add_vertex({0,0,0}); // Dummy vertex to attach properties to

    const auto prop_name = "pm_v_test_property";

    ASSERT_FALSE((OpenMesh::hasProperty<OpenMesh::VertexHandle, int>(mesh_, prop_name)));

    {
        auto prop = OpenMesh::getOrMakeProperty<OpenMesh::VertexHandle, int>(mesh_, prop_name);
        prop[vh] = 100;
        // End of scope, property persists
    }

    ASSERT_TRUE((OpenMesh::hasProperty<OpenMesh::VertexHandle, int>(mesh_, prop_name)));

    {
        // Since a property of the same name and type already exists, this refers to the existing property.
        auto prop = OpenMesh::getOrMakeProperty<OpenMesh::VertexHandle, int>(mesh_, prop_name);
        ASSERT_EQ(100, prop[vh]);
        prop[vh] = 200;
        // End of scope, property persists
    }

    ASSERT_TRUE((OpenMesh::hasProperty<OpenMesh::VertexHandle, int>(mesh_, prop_name)));

    {
        // Acquire non-owning handle to the property, knowing it exists
        auto prop = OpenMesh::getProperty<OpenMesh::VertexHandle, int>(mesh_, prop_name);
        ASSERT_EQ(200, prop[vh]);
    }

    ASSERT_TRUE((OpenMesh::hasProperty<OpenMesh::VertexHandle, int>(mesh_, prop_name)));

    {
        // Attempt to acquire non-owning handle for a non-existing property
        auto code_that_throws = [&](){
            OpenMesh::getProperty<OpenMesh::VertexHandle, int>(mesh_, "wrong_prop_name");
        };
        ASSERT_THROW(code_that_throws(), std::runtime_error);
    }

    ASSERT_TRUE((OpenMesh::hasProperty<OpenMesh::VertexHandle, int>(mesh_, prop_name)));
}


TEST_F(OpenMeshPropertyManager, property_copy_construction) {
  for (int i = 0; i < N_VERTICES_TIMING; ++i)
    mesh_.add_vertex(Mesh::Point());

  // unnamed
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_);
    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    auto  prop2 = prop1; // prop1 and prop2 should be two different properties with the same content

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13);
  }

  // named
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids");
    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    auto prop2 = prop1; // prop1 and prop2 should refere to the same property

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13);

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0);
  }
}

TEST_F(OpenMeshPropertyManager, property_move_construction) {
  for (int i = 0; i < N_VERTICES_TIMING; ++i)
    mesh_.add_vertex(Mesh::Point());

  // unnamed
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_);
    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    auto prop2 = std::move(prop1);
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "move constructing property from temporary took " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_FALSE(prop1.isValid()) << "prop1 should have been invalidated";

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13);
  }

  // named
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids");
    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    auto prop2 = std::move(prop1); // prop1 and prop2 should refere to the same property
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "move constructing from named took " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_TRUE(prop1.isValid()) << "named properties cannot be invalidated";

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "property is not valid anymore";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "did not copy property correctly";

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], 0);
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0);
  }
}


TEST_F(OpenMeshPropertyManager, property_copying_same_mesh) {

  for (int i = 0; i < N_VERTICES_TIMING; ++i)
    mesh_.add_vertex(Mesh::Point());

  // unnamed to unnamed
  {
    auto prop1 = OpenMesh::VProp<int>(3, mesh_);
    auto prop2 = OpenMesh::VProp<int>(0, mesh_);
    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], 3) << "Property not initialized correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = prop1;
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "copying property temporary to temporary took " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Temporary property got destroyed";

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], 0) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
  }

  // unnamed to named
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_);
    auto prop2 = OpenMesh::VProp<int>(0, mesh_, "ids");
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = prop1;
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "copying property temporary to named took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Temporary property got destroyed";

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    auto prop3 = OpenMesh::VProp<int>(mesh_, "ids");
    EXPECT_EQ(prop3[OpenMesh::VertexHandle(0)], -13) << "property with name 'ids' was not correctly changed";
  }

  // named to unnamed
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids2");
    auto prop2 = OpenMesh::VProp<int>(mesh_);
    prop2.set_range(mesh_.vertices(), 0);
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = prop1;
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "copying property named to temporary took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

  }

  // named to named (different names)
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids3");
    auto prop2 = OpenMesh::VProp<int>(mesh_, "ids4");
    prop2.set_range(mesh_.vertices(), 0);
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = prop1;
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "copying property named to named with different name took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
  }

  // named to named (same names)
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids5");
    auto prop2 = OpenMesh::VProp<int>(mesh_, "ids5");

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = prop1; // this should be a no op
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "copying property named to named with same name took " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    prop1.set_range(mesh_.vertices(), 42);

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], 42) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 42) << "Property not copied correctly";

    auto prop3 = OpenMesh::VProp<int>(mesh_, "ids5");
    EXPECT_EQ(prop3[OpenMesh::VertexHandle(0)], 42) << "Property not copied correctly";
  }

  {
    auto prop1 = OpenMesh::MProp<int>(mesh_);
    *prop1 = 43;
    auto prop2 = prop1;

    prop2 = prop1;

    prop2 = std::move(prop1);
  }
}


TEST_F(OpenMeshPropertyManager, property_moving_same_mesh) {

  for (int i = 0; i < N_VERTICES_TIMING; ++i)
    mesh_.add_vertex(Mesh::Point());

  // unnamed to unnamed
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_);
    auto prop2 = OpenMesh::VProp<int>(mesh_);
    prop2.set_range(mesh_.vertices(), 0);

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = std::move(prop1); // this should be cheap
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "moving property temporary to temporary took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_FALSE(prop1.isValid()) << "prop1 not invalidated after moving";

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
  }

  // unnamed to named
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_);
    auto prop2 = OpenMesh::VProp<int>(mesh_, "ids");
    prop2.set_range(mesh_.vertices(), 0);
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = std::move(prop1);
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "moving property temporary to named took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_FALSE(prop1.isValid()) << "prop1 not invalidated after moving";

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    auto prop3 = OpenMesh::VProp<int>(mesh_, "ids");
    EXPECT_EQ(prop3[OpenMesh::VertexHandle(0)], -13) << "property with name 'ids' was not correctly changed";
  }

  // named to unnamed
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids2");
    auto prop2 = OpenMesh::VProp<int>(mesh_);
    prop2.set_range(mesh_.vertices(), 0);
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = std::move(prop1); // moving named properties will not invalidate the property and will copy the data
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "moving property named to temporary took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_TRUE(prop1.isValid()) << "named prop1 should not be invalidated by moving";

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

  }

  // named to named (different names)
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids3");
    auto prop2 = OpenMesh::VProp<int>(mesh_, "ids4");
    prop2.set_range(mesh_.vertices(), 0);
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = std::move(prop1); // moving named properties will not invalidate the property and will copy the data
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "moving property named to named with different name took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_TRUE(prop1.isValid()) << "named prop1 should not be invalidated by moving";

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
  }

  // named to named (same names)
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids5");
    auto prop2 = OpenMesh::VProp<int>(mesh_, "ids5");

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = std::move(prop1); // this should be a no op
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "moving property named to named with same name took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_TRUE(prop1.isValid()) << "named prop1 should not be invalidated by moving";

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], 0) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not copied correctly";

    auto prop3 = OpenMesh::VProp<int>(mesh_, "ids5");
    EXPECT_EQ(prop3[OpenMesh::VertexHandle(0)], 0) << "Property not copied correctly";
  }
}



TEST_F(OpenMeshPropertyManager, property_copying_different_mesh) {

  for (int i = 0; i < N_VERTICES_TIMING; ++i)
    mesh_.add_vertex(Mesh::Point());

  auto copy = mesh_;
  for (int i = 0; i < 10; ++i)
    copy.add_vertex(Mesh::Point());

  // unnamed to unnamed
  {
    auto prop1 = OpenMesh::VProp<int>(3, mesh_);
    auto prop2 = OpenMesh::VProp<int>(0, copy);
    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], 3) << "Property not initialized correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = prop1;
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "copying property temporary to temporary took " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Temporary property got destroyed";

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], 0) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
    EXPECT_NO_FATAL_FAILURE(prop2[OpenMesh::VertexHandle(static_cast<int>(copy.n_vertices())-1)]) << "Property not correctly resized";
  }

  // unnamed to named
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_);
    auto prop2 = OpenMesh::VProp<int>(0, copy, "ids");
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = prop1;
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "copying property temporary to named took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Temporary property got destroyed";

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    auto prop3 = OpenMesh::VProp<int>(copy, "ids");
    EXPECT_EQ(prop3[OpenMesh::VertexHandle(0)], -13) << "property with name 'ids' was not correctly changed";
  }

  // named to unnamed
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids2");
    auto prop2 = OpenMesh::VProp<int>(copy);
    prop2.set_range(mesh_.vertices(), 0);
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = prop1;
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "copying property named to temporary took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

  }

  // named to named (different names)
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids3");
    auto prop2 = OpenMesh::VProp<int>(copy, "ids4");
    prop2.set_range(mesh_.vertices(), 0);
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = prop1;
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "copying property named to named with different name took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
  }

  // named to named (same names)
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids5");
    auto prop2 = OpenMesh::VProp<int>(copy, "ids5");

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = prop1; // this should be a no op
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "copying property named to named with same name took " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    prop1.set_range(mesh_.vertices(), 42);

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], 42) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    auto prop3 = OpenMesh::VProp<int>(mesh_, "ids5");
    EXPECT_EQ(prop3[OpenMesh::VertexHandle(0)], 42) << "Property not copied correctly";
    auto prop4 = OpenMesh::VProp<int>(copy, "ids5");
    EXPECT_EQ(prop4[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
  }
}


TEST_F(OpenMeshPropertyManager, property_moving_different_mesh) {

  for (int i = 0; i < N_VERTICES_TIMING; ++i)
    mesh_.add_vertex(Mesh::Point());

  auto copy = mesh_;
  for (int i = 0; i < 10; ++i)
    copy.add_vertex(Mesh::Point());

  // unnamed to unnamed
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_);
    auto prop2 = OpenMesh::VProp<int>(copy);
    prop2.set_range(mesh_.vertices(), 0);

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = std::move(prop1); // this should be cheap
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "moving property temporary to temporary took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_FALSE(prop1.isValid()) << "prop1 not invalidated after moving";

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
    EXPECT_NO_FATAL_FAILURE(prop2[OpenMesh::VertexHandle(static_cast<int>(copy.n_vertices())-1)]) << "Property not correctly resized";
  }

  // unnamed to named
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_);
    auto prop2 = OpenMesh::VProp<int>(copy, "ids");
    prop2.set_range(mesh_.vertices(), 0);
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = std::move(prop1);
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "moving property temporary to named took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_FALSE(prop1.isValid()) << "prop1 not invalidated after moving";

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    auto prop3 = OpenMesh::VProp<int>(copy, "ids");
    EXPECT_EQ(prop3[OpenMesh::VertexHandle(0)], -13) << "property with name 'ids' was not correctly changed";
  }

  // named to unnamed
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids2");
    auto prop2 = OpenMesh::VProp<int>(copy);
    prop2.set_range(mesh_.vertices(), 0);
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = std::move(prop1); // moving named properties will not invalidate the property and will copy the data
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "moving property named to temporary took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_TRUE(prop1.isValid()) << "named prop1 should not be invalidated by moving";

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

  }

  // named to named (different names)
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids3");
    auto prop2 = OpenMesh::VProp<int>(copy, "ids4");
    prop2.set_range(mesh_.vertices(), 0);
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], 0) << "Property not initialized correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = std::move(prop1); // moving named properties will not invalidate the property and will copy the data
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "moving property named to named with different name took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_TRUE(prop1.isValid()) << "named prop1 should not be invalidated by moving";

    prop1.set_range(mesh_.vertices(), 0);

    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
  }

  // named to named (same names)
  {
    auto prop1 = OpenMesh::VProp<int>(mesh_, "ids5");
    auto prop2 = OpenMesh::VProp<int>(copy, "ids5");

    auto prop6 = OpenMesh::Prop<OpenMesh::VertexHandle, int>(mesh_);
    prop6 = prop1;

    for (auto vh : mesh_.vertices())
      prop1[vh] = vh.idx()*2-13;

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
    prop2 = std::move(prop1); // should copy
    TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
    TIMING_OUTPUT(std::cout << "moving property named to named with same name took  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

    EXPECT_TRUE(prop1.isValid()) << "named prop1 should not be invalidated by moving";

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    prop1.set_range(mesh_.vertices(), 42);

    EXPECT_EQ(prop1[OpenMesh::VertexHandle(0)], 42) << "Property not copied correctly";
    EXPECT_EQ(prop2[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";

    auto prop3 = OpenMesh::VProp<int>(mesh_, "ids5");
    EXPECT_EQ(prop3[OpenMesh::VertexHandle(0)], 42) << "Property not copied correctly";
    auto prop4 = OpenMesh::VProp<int>(copy, "ids5");
    EXPECT_EQ(prop4[OpenMesh::VertexHandle(0)], -13) << "Property not copied correctly";
  }
}


TEST_F(OpenMeshPropertyManager, temporary_property_on_const_mesh) {

  const auto& const_ref = mesh_;

  auto cog = OpenMesh::FProp<Mesh::Point>(const_ref);
  auto points = OpenMesh::getPointsProperty(const_ref);

  for (auto fh : const_ref.faces())
    cog(fh) = fh.vertices().avg(points);

  auto cog_copy = cog;

  for (auto fh : const_ref.faces())
  {
    EXPECT_NE(&cog(fh), &cog_copy(fh)) << "Both properties point to the same memory";
    EXPECT_EQ(cog(fh), cog_copy(fh))   << "Property not copied correctly";
  }

  auto description = OpenMesh::MProp<std::string>(const_ref);
  description() = "Cool Const Mesh";

  std::cout << description(OpenMesh::MeshHandle(33)) << std::endl;

}


OpenMesh::VProp<int> get_id_prop(const OpenMesh::PolyConnectivity& mesh)
{
  TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)

  auto id_prop = OpenMesh::VProp<int>(mesh);
  for (auto vh : mesh.vertices())
    id_prop(vh) = vh.idx();

  TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
  TIMING_OUTPUT(std::cout << "Time spend in function:  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

  return id_prop;
}

TEST_F(OpenMeshPropertyManager, return_property_from_function) {

  for (int i = 0; i < N_VERTICES_TIMING; ++i)
    mesh_.add_vertex(Mesh::Point());

  TIMING_OUTPUT(auto t_start = std::chrono::high_resolution_clock::now();)
  auto id_p = get_id_prop(mesh_);
  TIMING_OUTPUT(auto t_end = std::chrono::high_resolution_clock::now();)
  TIMING_OUTPUT(std::cout << "Time spend around function  " << std::chrono::duration_cast<std::chrono::milliseconds>(t_end-t_start).count() << "ms" << std::endl;)

  for (auto vh : mesh_.vertices())
  {
    EXPECT_EQ(id_p(vh), vh.idx()) << "Property not returned correctly" << std::endl;
  }

}



}


#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
//#include <OpenMesh/Tools/Smoother/JacobiLaplaceSmootherT.hh>

namespace {

class OpenMesh_Poly : public OpenMeshBasePoly {

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

class OpenMesh_Triangle : public OpenMeshBase {

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

#if (defined(_MSC_VER) && (_MSC_VER >= 1800)) || __cplusplus > 199711L || defined(__GXX_EXPERIMENTAL_CXX0X__)

/*
 */
TEST_F(OpenMesh_Triangle, cpp11_initializer_test) {

  OpenMesh::Vec3d vec1 = { 1, 0, 0};
  OpenMesh::Vec3d vec2 = {0, 0.777971, 0.996969};


  EXPECT_EQ( dot(vec1,vec2) ,0.0 );

}


// ////////////////////////////////////////////////////////////////////
// C++11 Range tests
// ////////////////////////////////////////////////////////////////////


/**
 * @brief TEST_F test the vertexrange behaviour on trimeshes
 */
TEST_F(OpenMesh_Triangle, cpp11_vertexrange_test) {
  //check empty vertexrange
  mesh_.clear();
  for(Mesh::VertexHandle t : mesh_.vertices())
  {
    FAIL() << "The Vertexrange for an empty Mesh is not empty";
    EXPECT_TRUE(t.is_valid()); // Just so we don't get an unused variable warning.
  }

  //add vertices to mesh
  mesh_.add_vertex(Mesh::Point(0, 1, 0));
  mesh_.add_vertex(Mesh::Point(1, 0, 0));
  mesh_.add_vertex(Mesh::Point(2, 1, 0));
  mesh_.add_vertex(Mesh::Point(0,-1, 0));
  mesh_.add_vertex(Mesh::Point(2,-1, 0));

  //check nonempty mesh vertexrange
  int iteration_counter = 0;
  for(Mesh::VertexHandle t : mesh_.vertices())
  {
    EXPECT_TRUE(t.is_valid());
    ++iteration_counter;
  }
  EXPECT_EQ(iteration_counter,5);
}

/**
 * @brief TEST_F test the vertexrange behaviour on polymeshes
 */
TEST_F(OpenMesh_Poly, cpp11_vertexrange_test) {
  //check empty vertexrange
  mesh_.clear();
  for(PolyMesh::VertexHandle t : mesh_.vertices())
  {
    FAIL() << "The Vertexrange for an empty Mesh is not empty";
    EXPECT_TRUE(t.is_valid()); // Just so we don't get an unused variable warning.
  }

  //add vertices to mesh
  mesh_.add_vertex(PolyMesh::Point(0, 1, 0));
  mesh_.add_vertex(PolyMesh::Point(1, 0, 0));
  mesh_.add_vertex(PolyMesh::Point(2, 1, 0));
  mesh_.add_vertex(PolyMesh::Point(0,-1, 0));
  mesh_.add_vertex(PolyMesh::Point(2,-1, 0));

  //check nonempty mesh vertexrange
  int iteration_counter = 0;
  for(PolyMesh::VertexHandle t : mesh_.vertices())
  {
    EXPECT_TRUE(t.is_valid());
    ++iteration_counter;
  }
  EXPECT_EQ(iteration_counter,5);
}

/**
 * @brief TEST_F test vvrange behaviour on trimeshes
 */
TEST_F(OpenMesh_Triangle, cpp11_vvrange_test) {
  //check empty vv_range
  mesh_.clear();
  Mesh::VertexHandle vhandle[5];
  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  for(Mesh::VertexHandle t : mesh_.vv_range(vhandle[0]))
  {
    FAIL() << "The vvrange for a single vertex in a TriMesh is not empty";
    EXPECT_TRUE(t.is_valid()); // Just so we don't get an unused variable warning.
  }

  //add more vertices
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 0));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0,-1, 0));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(2,-1, 0));

  // Add 4 faces
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

  //check nonempty vvrange
  int iteration_counter = 0;
  for(Mesh::VertexHandle t : mesh_.vv_range(vhandle[1]))
  {
    EXPECT_TRUE(t.is_valid());
    ++iteration_counter;
  }
  EXPECT_EQ(iteration_counter,4);
}

/**
 * @brief TEST_F test the vvrange behaviour on polymeshes
 */
TEST_F(OpenMesh_Poly, cpp11_vvrange_test) {
  //check empty vv_range
  mesh_.clear();
  Mesh::VertexHandle vhandle[5];
  vhandle[0] = mesh_.add_vertex(PolyMesh::Point(0, 1, 0));
  for(PolyMesh::VertexHandle t : mesh_.vv_range(vhandle[0]))
  {
    FAIL() << "The vvrange for a single vertex in a PolyMesh is not empty";
    EXPECT_TRUE(t.is_valid()); // Just so we don't get an unused variable warning.
  }

  //add more vertices
  vhandle[1] = mesh_.add_vertex(PolyMesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(PolyMesh::Point(2, 1, 0));
  vhandle[3] = mesh_.add_vertex(PolyMesh::Point(0,-1, 0));
  vhandle[4] = mesh_.add_vertex(PolyMesh::Point(2,-1, 0));

  // Add 4 faces
  std::vector<PolyMesh::VertexHandle> face_vhandles;

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

  //check nonempty vvrange
  int iteration_counter = 0;
  for(PolyMesh::VertexHandle t : mesh_.vv_range(vhandle[1]))
  {
    EXPECT_TRUE(t.is_valid());
    ++iteration_counter;
  }
  EXPECT_EQ(iteration_counter,4);
}


/**
 * @brief Test combined vertex iterator and vertex vertex iter.
 */
TEST_F(OpenMesh_Triangle, cpp11_test_enumerate_combined_run) {
  //check empty vv_range
  mesh_.clear();
  Mesh::VertexHandle vhandle[5];
  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  for(Mesh::VertexHandle t : mesh_.vv_range(vhandle[0]))
  {
    FAIL() << "The vvrange for a single vertex in a TriMesh is not empty";
    EXPECT_TRUE(t.is_valid()); // Just so we don't get an unused variable warning.
  }

  //add more vertices
  vhandle[1] = mesh_.add_vertex(Mesh::Point(1, 0, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(2, 1, 3));
  vhandle[3] = mesh_.add_vertex(Mesh::Point(0,-1, 4));
  vhandle[4] = mesh_.add_vertex(Mesh::Point(2,3, 5));

  // Add 4 faces
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

  Mesh::Point p = Mesh::Point(0,0,0);
  for ( auto vh : mesh_.vertices() ) {

    for ( auto vv_it : mesh_.vv_range(vh) ) {
       p += mesh_.point(vv_it);
    }

  }

  EXPECT_EQ(p[0],16);
  EXPECT_EQ(p[1],12);
  EXPECT_EQ(p[2],36);

}


#endif

}

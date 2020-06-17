
#ifdef ENABLE_EIGEN3_TEST

#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <iostream>

#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalDeviationT.hh>

#include <OpenMesh/Core/Geometry/EigenVectorT.hh>

struct EigenTraits : OpenMesh::DefaultTraits {
    using Point = Eigen::Vector3d;
    using Normal = Eigen::Vector3d;

    using TexCoord2D = Eigen::Vector2d;
};

using EigenTriMesh = OpenMesh::TriMesh_ArrayKernelT<EigenTraits>;

namespace {


class OpenMeshEigenTest : public testing::Test {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
            
            // Do some initial stuff with the member data here...
        }

        // This function is called after all tests are through
        virtual void TearDown() {


        }

        EigenTriMesh mesh_;
};

TEST_F(OpenMeshEigenTest, Test_external_vectorize) {


  EigenTriMesh::Normal normal;

  vectorize(normal,2);

  EXPECT_EQ(normal[0],2.0f ) << "Wrong x value";
  EXPECT_EQ(normal[1],2.0f ) << "Wrong y value";
  EXPECT_EQ(normal[2],2.0f ) << "Wrong z value";


}

TEST_F(OpenMeshEigenTest, Test_external_norm) {


  EigenTriMesh::Normal normal(1,0,0);

  EigenTriMesh::Scalar result = norm(normal);

  EXPECT_EQ(result,1.0f ) << "Wrong norm";

  normal = EigenTriMesh::Normal(2,0,0);

  result = norm(normal);

  EXPECT_EQ(result,2.0f ) << "Wrong norm";
}

TEST_F(OpenMeshEigenTest, Test_external_sqrnorm) {


  EigenTriMesh::Normal normal(1,0,0);

  EigenTriMesh::Scalar result = sqrnorm(normal);

  EXPECT_EQ(result,1.0f ) << "Wrong norm";

  normal = EigenTriMesh::Normal(2,0,0);

  result = sqrnorm(normal);

  EXPECT_EQ(result,4.0f ) << "Wrong norm";
}

TEST_F(OpenMeshEigenTest, Test_external_normalize) {


  EigenTriMesh::Normal normal(2,0,0);

  normalize(normal);

  EXPECT_EQ(norm(normal),1.0f ) << "Wrong norm after normalization";

  normal = EigenTriMesh::Normal(2,6,9);

  normalize(normal);

  EXPECT_EQ(norm(normal),1.0f ) << "Wrong norm after normalization";

}

TEST_F(OpenMeshEigenTest, Test_external_cross_Product) {


  EigenTriMesh::Normal normal1(1,0,0);
  EigenTriMesh::Normal normal2(1,1,0);

  EigenTriMesh::Normal result = cross(normal1,normal2);

  EXPECT_EQ(result[0],0.0f ) << "Wrong result x direction";
  EXPECT_EQ(result[1],0.0f ) << "Wrong result y direction";
  EXPECT_EQ(result[2],1.0f ) << "Wrong result z direction";
}

TEST_F(OpenMeshEigenTest, Test_external_dot_Product) {


  EigenTriMesh::Normal normal1(1,0,0);
  EigenTriMesh::Normal normal2(1,1,0);
  EigenTriMesh::Normal normal3(1,1,1);
  EigenTriMesh::Normal normal4(2,4,6);

  EigenTriMesh::Scalar result  = dot(normal1,normal2);
  EigenTriMesh::Scalar result1 = dot(normal3,normal4);

  EXPECT_EQ(result,1.0f ) << "Wrong dot product";
  EXPECT_EQ(result1,12.0f ) << "Wrong dot product";

}


TEST_F(OpenMeshEigenTest, Test_Basic_setup) {

  // Add some vertices
  EigenTriMesh::VertexHandle vhandle[4];

  vhandle[0] = mesh_.add_vertex(EigenTriMesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(EigenTriMesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(EigenTriMesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(EigenTriMesh::Point(1, 0, 0));

  // Add two faces
  std::vector<EigenTriMesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);

  mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  mesh_.add_face(face_vhandles);


  EXPECT_EQ(mesh_.n_faces(),2u) << "Wrong number of faces";

}

TEST_F(OpenMeshEigenTest, test_normal_computation) {

  // Add some vertices
  EigenTriMesh::VertexHandle vhandle[4];

  mesh_.request_vertex_normals();
  mesh_.request_face_normals();

  vhandle[0] = mesh_.add_vertex(EigenTriMesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(EigenTriMesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(EigenTriMesh::Point(1, 1, 0));
  vhandle[3] = mesh_.add_vertex(EigenTriMesh::Point(1, 0, 0));

  // Add two faces
  std::vector<EigenTriMesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);

  EigenTriMesh::FaceHandle face1 = mesh_.add_face(face_vhandles);

  face_vhandles.clear();

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  EigenTriMesh::FaceHandle face2 = mesh_.add_face(face_vhandles);

  mesh_.update_face_normals();


  EXPECT_EQ(mesh_.n_faces(),2u) << "Wrong number of faces";

  EigenTriMesh::Normal normal = mesh_.normal(face1);

  EXPECT_EQ(normal[0],0.0f ) << "Wrong normal x direction";
  EXPECT_EQ(normal[1],0.0f ) << "Wrong normal y direction";
  EXPECT_EQ(normal[2],1.0f ) << "Wrong normal z direction";

  normal = mesh_.normal(face2);

  EXPECT_EQ(normal[0],0.0f ) << "Wrong normal x direction";
  EXPECT_EQ(normal[1],0.0f ) << "Wrong normal y direction";
  EXPECT_EQ(normal[2],1.0f ) << "Wrong normal z direction";

}

/* Just load a simple mesh file in obj format and count whether
*  the right number of entities has been loaded. Afterwards recompute
*  normals
*/
TEST_F(OpenMeshEigenTest, LoadSimpleOFFFile) {

   mesh_.clear();

   bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.off");

   EXPECT_TRUE(ok);

   EXPECT_EQ(7526u , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
   EXPECT_EQ(22572u, mesh_.n_edges()) << "The number of loaded edges is not correct!";
   EXPECT_EQ(15048u, mesh_.n_faces()) << "The number of loaded faces is not correct!";

   mesh_.update_normals();
}

// Test decimation with Eigen as vector type
TEST_F(OpenMeshEigenTest, Decimater) {
   mesh_.clear();

   bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.off");

   EXPECT_TRUE(ok);

   EXPECT_EQ(7526u , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
   EXPECT_EQ(22572u, mesh_.n_edges()) << "The number of loaded edges is not correct!";
   EXPECT_EQ(15048u, mesh_.n_faces()) << "The number of loaded faces is not correct!";

   mesh_.update_normals();

   OpenMesh::Decimater::DecimaterT<EigenTriMesh> decimater(mesh_);
   OpenMesh::Decimater::ModQuadricT<EigenTriMesh>::Handle hModQuadric;                   // use a quadric module
   OpenMesh::Decimater::ModNormalDeviationT<EigenTriMesh>::Handle hModNormalDeviation;   // also use normal deviation module as binary check
   decimater.add(hModQuadric);
   decimater.add(hModNormalDeviation);
   decimater.module(hModQuadric).unset_max_err();
   decimater.module(hModNormalDeviation).set_normal_deviation(15);
   decimater.initialize();
   size_t removedVertices = decimater.decimate_to(8);
   mesh_.garbage_collection();

   EXPECT_EQ(6998u, removedVertices)    << "The number of remove vertices is not correct!";
   EXPECT_EQ( 528u, mesh_.n_vertices()) << "The number of vertices after decimation is not correct!";
   EXPECT_EQ(1578u, mesh_.n_edges())    << "The number of edges after decimation is not correct!";
   EXPECT_EQ(1052u, mesh_.n_faces())    << "The number of faces after decimation is not correct!";
}

}

#endif

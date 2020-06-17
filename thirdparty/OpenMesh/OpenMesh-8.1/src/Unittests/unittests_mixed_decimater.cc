
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <OpenMesh/Tools/Decimater/MixedDecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalFlippingT.hh>

namespace {

class OpenMeshMixedDecimater : public OpenMeshBase {

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

/*
 */
TEST_F(OpenMeshMixedDecimater, DecimateMesh80PercentMc) {

  bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.off");

  ASSERT_TRUE(ok);

  typedef OpenMesh::Decimater::MixedDecimaterT< Mesh >  Decimater;
  typedef OpenMesh::Decimater::ModQuadricT< Mesh >::Handle HModQuadric;

  Decimater decimaterDBG(mesh_);
  HModQuadric hModQuadricDBG;
  decimaterDBG.add( hModQuadricDBG );
  decimaterDBG.initialize();
  size_t removedVertices = 0;
  removedVertices = decimaterDBG.decimate_to(5000,0.8f);
                    decimaterDBG.mesh().garbage_collection();

  EXPECT_EQ(2526u, removedVertices) << "The number of remove vertices is not correct!";
  EXPECT_EQ(5000u, mesh_.n_vertices()) << "The number of vertices after decimation is not correct!";
  EXPECT_EQ(14994u, mesh_.n_edges()) << "The number of edges after decimation is not correct!";
  EXPECT_EQ(9996u, mesh_.n_faces()) << "The number of faces after decimation is not correct!";
}

TEST_F(OpenMeshMixedDecimater, DecimateMeshToFaceVerticesLimit) {

  bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.off");

  ASSERT_TRUE(ok);

  typedef OpenMesh::Decimater::MixedDecimaterT< Mesh >  Decimater;
  typedef OpenMesh::Decimater::ModQuadricT< Mesh >::Handle HModQuadric;

  Decimater decimaterDBG(mesh_);
  HModQuadric hModQuadricDBG;
  decimaterDBG.add( hModQuadricDBG );
  decimaterDBG.initialize();
  size_t removedVertices = 0;
  removedVertices = decimaterDBG.decimate_to_faces(5000, 8000, 0.7f);
                    decimaterDBG.mesh().garbage_collection();

  EXPECT_EQ(2526u, removedVertices) << "The number of remove vertices is not correct!";
  EXPECT_EQ(5000u, mesh_.n_vertices()) << "The number of vertices after decimation is not correct!";
  EXPECT_EQ(14994u, mesh_.n_edges()) << "The number of edges after decimation is not correct!";
  EXPECT_EQ(9996u, mesh_.n_faces()) << "The number of faces after decimation is not correct!";
}

TEST_F(OpenMeshMixedDecimater, DecimateMeshToFaceFaceLimit) {

  bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.off");

  ASSERT_TRUE(ok);

  typedef OpenMesh::Decimater::MixedDecimaterT< Mesh >  Decimater;
  typedef OpenMesh::Decimater::ModQuadricT< Mesh >::Handle HModQuadric;

  Decimater decimaterDBG(mesh_);
  HModQuadric hModQuadricDBG;
  decimaterDBG.add( hModQuadricDBG );
  decimaterDBG.initialize();
  size_t removedVertices = 0;
  removedVertices = decimaterDBG.decimate_to_faces(4500, 9996, 0.7f);
                    decimaterDBG.mesh().garbage_collection();

  EXPECT_EQ(2526u, removedVertices) << "The number of remove vertices is not correct!";
  EXPECT_EQ(5000u, mesh_.n_vertices()) << "The number of vertices after decimation is not correct!";
  EXPECT_EQ(14994u, mesh_.n_edges()) << "The number of edges after decimation is not correct!";
  EXPECT_EQ(9996u, mesh_.n_faces()) << "The number of faces after decimation is not correct!";
}

class UnittestObserver : public OpenMesh::Decimater::Observer
{
    size_t notifies_;
    size_t all_steps_;
public:
    explicit UnittestObserver(size_t _steps) :Observer(_steps), notifies_(0), all_steps_(0) {}

    void notify(size_t _step)
    {
        ++notifies_;
        all_steps_ = _step;
    }
    bool abort() const
    {
        return all_steps_ >= 2526u;
    }

    size_t countedNotifies()
    {
        return notifies_;
    }
};

TEST_F(OpenMeshMixedDecimater, DecimateMeshStoppedByObserver) {

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.off");

    ASSERT_TRUE(ok);

    typedef OpenMesh::Decimater::MixedDecimaterT< Mesh >  Decimater;
    typedef OpenMesh::Decimater::ModQuadricT< Mesh >::Handle HModQuadric;

    Decimater decimaterDBG(mesh_);
    HModQuadric hModQuadricDBG;
    decimaterDBG.add(hModQuadricDBG);

    decimaterDBG.module(hModQuadricDBG).unset_max_err();

    decimaterDBG.initialize();
    UnittestObserver obs(2);
    decimaterDBG.set_observer(&obs);
    size_t removedVertices = 0;
    removedVertices = decimaterDBG.decimate_to_faces(0, 0);
    decimaterDBG.mesh().garbage_collection();

    EXPECT_TRUE(obs.abort()) << "Observer did not abort the decimater!";
    EXPECT_EQ(obs.countedNotifies(), 2526u / 2u) << "Observer did not get the right amount of notifications!";

    EXPECT_EQ(2526u, removedVertices) << "The number of remove vertices is not correct!";
    EXPECT_EQ(5000u, mesh_.n_vertices()) << "The number of vertices after decimation is not correct!";
    EXPECT_EQ(14994u, mesh_.n_edges()) << "The number of edges after decimation is not correct!";
    EXPECT_EQ(9996u, mesh_.n_faces()) << "The number of faces after decimation is not correct!";
}
}

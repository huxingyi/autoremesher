#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>


namespace {

class OpenMeshReadWritePLY : public OpenMeshBase {

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
 * Just load a point file in ply format and count whether
 * the right number of entities has been loaded.
 */
TEST_F(OpenMeshReadWritePLY, LoadSimplePointPLYFileWithBadEncoding) {

    mesh_.clear();

    bool ok = OpenMesh::IO::read_mesh(mesh_, "pointCloudBadEncoding.ply");

    EXPECT_TRUE(ok) << "Unable to load pointCloudBadEncoding.ply";

    EXPECT_EQ(10u , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(0u  , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(0u  , mesh_.n_faces()) << "The number of loaded faces is not correct!";
}

/*
 * Just load a point file in ply format and count whether
 * the right number of entities has been loaded.
 */
TEST_F(OpenMeshReadWritePLY, LoadSimplePointPLYFileWithGoodEncoding) {

    mesh_.clear();

    bool ok = OpenMesh::IO::read_mesh(mesh_, "pointCloudGoodEncoding.ply");

    EXPECT_TRUE(ok) << "Unable to load pointCloudGoodEncoding.ply";

    EXPECT_EQ(10u , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(0u  , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(0u  , mesh_.n_faces()) << "The number of loaded faces is not correct!";
}

/*
 * Just load a ply
 */
TEST_F(OpenMeshReadWritePLY, LoadSimplePLY) {

    mesh_.clear();

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal.ply");

    EXPECT_TRUE(ok) << "Unable to load cube-minimal.ply";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces()) << "The number of loaded faces is not correct!";

}

/*
 * Load a ply ascii file without a newline at the end of the file
 *
 */
TEST_F(OpenMeshReadWritePLY, LoadSimplePLYNoEndl) {

    mesh_.clear();

    bool ok = OpenMesh::IO::read_mesh(mesh_, "sphere840.ply");

    EXPECT_TRUE(ok) << "Unable to load sphere840.ply";

    EXPECT_EQ(422u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(1260u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(840u , mesh_.n_faces()) << "The number of loaded faces is not correct!";

}

/*
 * Just load a ply file and set vertex color option before loading
 */
TEST_F(OpenMeshReadWritePLY, LoadSimplePLYForceVertexColorsAlthoughNotAvailable) {

  mesh_.clear();

  mesh_.request_vertex_colors();

  std::string file_name = "cube-minimal.ply";

  OpenMesh::IO::Options options;
  options += OpenMesh::IO::Options::VertexColor;

  bool ok = OpenMesh::IO::read_mesh(mesh_, file_name,options);

  EXPECT_TRUE(ok) << file_name;

  EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
  EXPECT_EQ(18u , mesh_.n_edges())     << "The number of loaded edges is not correct!";
  EXPECT_EQ(12u , mesh_.n_faces())     << "The number of loaded faces is not correct!";
  EXPECT_EQ(36u , mesh_.n_halfedges())  << "The number of loaded halfedges is not correct!";

  EXPECT_FALSE(options.vertex_has_normal()) << "Wrong user options are returned!";
  EXPECT_FALSE(options.vertex_has_texcoord()) << "Wrong user options are returned!";
  EXPECT_FALSE(options.vertex_has_color()) << "Wrong user options are returned!";
}

/*
 * Just load a ply file of a cube with vertex colors
 */
TEST_F(OpenMeshReadWritePLY, LoadSimplePLYWithVertexColors) {

    mesh_.clear();

    mesh_.request_vertex_colors();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::VertexColor;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-vertexColors.ply",options);

    EXPECT_TRUE(ok) << "Unable to load cube-minimal-vertexColors.ply";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces())    << "The number of loaded faces is not correct!";

#ifdef TEST_DOUBLE_TRAITS
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#else
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#endif

    EXPECT_FALSE(options.vertex_has_normal()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.vertex_has_texcoord()) << "Wrong user options are returned!";
    EXPECT_TRUE(options.vertex_has_color()) << "Wrong user options are returned!";

    mesh_.release_vertex_colors();
}

/*
 * Just load a ply file of a cube with vertex colors
 */
TEST_F(OpenMeshReadWritePLY, LoadPLYFromMeshLabWithVertexColors) {

    mesh_.clear();

    mesh_.request_vertex_colors();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::VertexColor;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "meshlab.ply",options);

    EXPECT_TRUE(ok) << "Unable to load meshlab.ply";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces())    << "The number of loaded faces is not correct!";

#ifdef TEST_DOUBLE_TRAITS
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#else
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#endif

    EXPECT_FALSE(options.vertex_has_normal()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.vertex_has_texcoord()) << "Wrong user options are returned!";
    EXPECT_TRUE(options.vertex_has_color()) << "Wrong user options are returned!";

    mesh_.release_vertex_colors();
}

/*
 * Just read and write a binary ply file of a cube with vertex colors
 */
TEST_F(OpenMeshReadWritePLY, WriteAndReadBinaryPLYWithVertexColors) {

    mesh_.clear();

    mesh_.request_vertex_colors();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::VertexColor;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "meshlab.ply",options);

    EXPECT_TRUE(ok) << "Unable to load meshlab.ply";

    options += OpenMesh::IO::Options::Binary;

    ok = OpenMesh::IO::write_mesh(mesh_, "meshlab_binary.ply",options);
    EXPECT_TRUE(ok) << "Unable to write meshlab_binary.ply";

    mesh_.clear();
    ok = OpenMesh::IO::read_mesh(mesh_, "meshlab_binary.ply",options);
    EXPECT_TRUE(ok) << "Unable to load meshlab_binary.ply";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces())    << "The number of loaded faces is not correct!";

#ifdef TEST_DOUBLE_TRAITS
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#else
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#endif

    EXPECT_FALSE(options.vertex_has_normal()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.vertex_has_texcoord()) << "Wrong user options are returned!";
    EXPECT_TRUE(options.vertex_has_color()) << "Wrong user options are returned!";

    mesh_.release_vertex_colors();
}

/*
 * Just read and write a ply file of a cube with float vertex colors
 */
TEST_F(OpenMeshReadWritePLY, WriteAndReadPLYWithFloatVertexColors) {

    mesh_.clear();

    mesh_.request_vertex_colors();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::VertexColor;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "meshlab.ply",options);

    EXPECT_TRUE(ok) << "Unable to load meshlab.ply";

    options += OpenMesh::IO::Options::ColorFloat;

    ok = OpenMesh::IO::write_mesh(mesh_, "meshlab_float.ply",options);
    EXPECT_TRUE(ok) << "Unable to write meshlab_float.ply";

    mesh_.clear();
    ok = OpenMesh::IO::read_mesh(mesh_, "meshlab_float.ply",options);
    EXPECT_TRUE(ok) << "Unable to load meshlab_float.ply";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces())    << "The number of loaded faces is not correct!";

#ifdef TEST_DOUBLE_TRAITS
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#else
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#endif

    EXPECT_FALSE(options.vertex_has_normal()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.vertex_has_texcoord()) << "Wrong user options are returned!";
    EXPECT_TRUE(options.vertex_has_color()) << "Wrong user options are returned!";
    EXPECT_TRUE(options.color_is_float()) << "Wrong user options are returned!";

    mesh_.release_vertex_colors();
}

/*
 * Just read and write a binary ply file of a cube with float vertex colors
 */
TEST_F(OpenMeshReadWritePLY, WriteAndReadBinaryPLYWithFloatVertexColors) {

    mesh_.clear();

    mesh_.request_vertex_colors();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::VertexColor;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "meshlab.ply",options);

    EXPECT_TRUE(ok) << "Unable to load meshlab.ply";

    options += OpenMesh::IO::Options::ColorFloat;
    options += OpenMesh::IO::Options::Binary;

    ok = OpenMesh::IO::write_mesh(mesh_, "meshlab_binary_float.ply",options);
    EXPECT_TRUE(ok) << "Unable to write meshlab_binary_float.ply";

    mesh_.clear();
    ok = OpenMesh::IO::read_mesh(mesh_, "meshlab_binary_float.ply",options);
    EXPECT_TRUE(ok) << "Unable to load meshlab_binary_float.ply";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces())    << "The number of loaded faces is not correct!";

#ifdef TEST_DOUBLE_TRAITS
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_FLOAT_EQ(0.0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#else
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#endif

    EXPECT_FALSE(options.vertex_has_normal()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.vertex_has_texcoord()) << "Wrong user options are returned!";
    EXPECT_TRUE(options.vertex_has_color()) << "Wrong user options are returned!";
    EXPECT_TRUE(options.color_is_float()) << "Wrong user options are returned!";
    EXPECT_TRUE(options.is_binary()) << "Wrong user options are returned!";

    mesh_.release_vertex_colors();
}

/*
 * Just load a ply file of a cube with vertex texCoords
 */
TEST_F(OpenMeshReadWritePLY, LoadSimplePLYWithTexCoords) {

    mesh_.clear();

    mesh_.request_vertex_texcoords2D();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::VertexTexCoord;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-texCoords.ply",options);

    EXPECT_TRUE(ok) << "Unable to load cube-minimal-texCoords.ply";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces())    << "The number of loaded faces is not correct!";

    EXPECT_EQ(10,   mesh_.texcoord2D(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(10,   mesh_.texcoord2D(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";

    EXPECT_EQ(6,   mesh_.texcoord2D(mesh_.vertex_handle(2))[0] ) << "Wrong vertex color at vertex 2 component 0";
    EXPECT_EQ(6,   mesh_.texcoord2D(mesh_.vertex_handle(2))[1] ) << "Wrong vertex color at vertex 2 component 1";

    EXPECT_EQ(9,   mesh_.texcoord2D(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(9,   mesh_.texcoord2D(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";

    EXPECT_EQ(12,   mesh_.texcoord2D(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(12,   mesh_.texcoord2D(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";


    EXPECT_FALSE(options.vertex_has_normal()) << "Wrong user options are returned!";
    EXPECT_TRUE(options.vertex_has_texcoord()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.vertex_has_color()) << "Wrong user options are returned!";

    mesh_.release_vertex_texcoords2D();
}

/*
 * Just load a ply with normals, ascii mode
 */
TEST_F(OpenMeshReadWritePLY, LoadSimplePLYWithNormals) {

    mesh_.clear();

    mesh_.request_vertex_normals();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::VertexNormal;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-normals.ply", options);

    EXPECT_TRUE(ok) << "Unable to load cube-minimal-normals.ply";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces()) << "The number of loaded faces is not correct!";

    EXPECT_TRUE(options.vertex_has_normal()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.vertex_has_texcoord()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.vertex_has_color()) << "Wrong user options are returned!";


    EXPECT_EQ(0, mesh_.normal(mesh_.vertex_handle(0))[0] ) << "Wrong normal at vertex 0 component 0";
    EXPECT_EQ(0, mesh_.normal(mesh_.vertex_handle(0))[1] ) << "Wrong normal at vertex 0 component 1";
    EXPECT_EQ(1, mesh_.normal(mesh_.vertex_handle(0))[2] ) << "Wrong normal at vertex 0 component 2";

    EXPECT_EQ(1, mesh_.normal(mesh_.vertex_handle(3))[0] ) << "Wrong normal at vertex 3 component 0";
    EXPECT_EQ(0, mesh_.normal(mesh_.vertex_handle(3))[1] ) << "Wrong normal at vertex 3 component 1";
    EXPECT_EQ(0, mesh_.normal(mesh_.vertex_handle(3))[2] ) << "Wrong normal at vertex 3 component 2";

    EXPECT_EQ(1, mesh_.normal(mesh_.vertex_handle(4))[0] ) << "Wrong normal at vertex 4 component 0";
    EXPECT_EQ(0, mesh_.normal(mesh_.vertex_handle(4))[1] ) << "Wrong normal at vertex 4 component 1";
    EXPECT_EQ(1, mesh_.normal(mesh_.vertex_handle(4))[2] ) << "Wrong normal at vertex 4 component 2";

    EXPECT_EQ(1, mesh_.normal(mesh_.vertex_handle(7))[0] ) << "Wrong normal at vertex 7 component 0";
    EXPECT_EQ(1, mesh_.normal(mesh_.vertex_handle(7))[1] ) << "Wrong normal at vertex 7 component 1";
    EXPECT_EQ(2, mesh_.normal(mesh_.vertex_handle(7))[2] ) << "Wrong normal at vertex 7 component 2";

    mesh_.release_vertex_normals();

}

/*
 * Just load a ply with custom properties, ascii mode
 */
TEST_F(OpenMeshReadWritePLY, LoadSimplePLYWithCustomProps) {

    PolyMesh mesh;

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::Custom;

    bool ok = OpenMesh::IO::read_mesh(mesh, "cube-minimal-custom_props.ply", options);

    EXPECT_TRUE(ok) << "Unable to load cube-minimal-custom_props.ply";

    EXPECT_EQ(8u  , mesh.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(12u , mesh.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(6u , mesh.n_faces()) << "The number of loaded faces is not correct!";

    OpenMesh::VPropHandleT<float> qualityProp;
    OpenMesh::VPropHandleT<unsigned int> indexProp;
    ASSERT_TRUE(mesh.get_property_handle(qualityProp,"quality")) << "Could not access quality property";
    ASSERT_TRUE(mesh.get_property_handle(indexProp,"index")) << "Could not access index property";

    //check index property
    for (unsigned i = 0; i < mesh.n_vertices(); ++i)
      EXPECT_EQ(i ,mesh.property(indexProp,OpenMesh::VertexHandle(i))) << "Vertex index at vertex " << i << " is wrong";

    //check quality property
    EXPECT_EQ(1.f,mesh.property(qualityProp,OpenMesh::VertexHandle(0))) << "Wrong quality value at Vertex 0";
    EXPECT_EQ(0.5f,mesh.property(qualityProp,OpenMesh::VertexHandle(1))) << "Wrong quality value at Vertex 1";
    EXPECT_EQ(0.7f,mesh.property(qualityProp,OpenMesh::VertexHandle(2))) << "Wrong quality value at Vertex 2";
    EXPECT_EQ(1.f,mesh.property(qualityProp,OpenMesh::VertexHandle(3))) << "Wrong quality value at Vertex 3";
    EXPECT_EQ(0.1f,mesh.property(qualityProp,OpenMesh::VertexHandle(4))) << "Wrong quality value at Vertex 4";
    EXPECT_EQ(0.f,mesh.property(qualityProp,OpenMesh::VertexHandle(5))) << "Wrong quality value at Vertex 5";
    EXPECT_EQ(2.f,mesh.property(qualityProp,OpenMesh::VertexHandle(6))) << "Wrong quality value at Vertex 6";
    EXPECT_EQ(5.f,mesh.property(qualityProp,OpenMesh::VertexHandle(7))) << "Wrong quality value at Vertex 7";

    //check for custom list properties

    OpenMesh::VPropHandleT< std::vector<int> > testValues;
    ASSERT_TRUE(mesh.get_property_handle(testValues,"test_values")) << "Could not access texcoords per face";

    EXPECT_EQ(2u,mesh.property(testValues,OpenMesh::VertexHandle(0)).size()) << "Wrong verctor size";

    EXPECT_EQ(1,mesh.property(testValues,OpenMesh::VertexHandle(0))[0]) << "Wrong list value at Vertex 0";
    EXPECT_EQ(4,mesh.property(testValues,OpenMesh::VertexHandle(1))[1]) << "Wrong list value at Vertex 1";
    EXPECT_EQ(5,mesh.property(testValues,OpenMesh::VertexHandle(2))[0]) << "Wrong list value at Vertex 2";
    EXPECT_EQ(8,mesh.property(testValues,OpenMesh::VertexHandle(3))[1]) << "Wrong list value at Vertex 3";
    EXPECT_EQ(9,mesh.property(testValues,OpenMesh::VertexHandle(4))[0]) << "Wrong list value at Vertex 4";
    EXPECT_EQ(12,mesh.property(testValues,OpenMesh::VertexHandle(5))[1]) << "Wrong list value at Vertex 5";
    EXPECT_EQ(13,mesh.property(testValues,OpenMesh::VertexHandle(6))[0]) << "Wrong list value at Vertex 6";
    EXPECT_EQ(16,mesh.property(testValues,OpenMesh::VertexHandle(7))[1]) << "Wrong list value at Vertex 7";

    OpenMesh::FPropHandleT< std::vector<float> > texCoordsPerFace;
    ASSERT_TRUE(mesh.get_property_handle(texCoordsPerFace,"texcoords")) << "Could not access texcoords per face";

    for (Mesh::FaceIter f_iter = mesh.faces_begin(); f_iter != mesh.faces_end(); ++f_iter)
    {
      EXPECT_EQ(8u, mesh.property(texCoordsPerFace, *f_iter).size()) << "Texcoords per face container has wrong size on face: " << f_iter->idx();
      if (!mesh.property(texCoordsPerFace, *f_iter).empty())
      {
        EXPECT_EQ(1.0, mesh.property(texCoordsPerFace, *f_iter)[0]) << "Texcoords wrong on index 0 with face: " << f_iter->idx();
        EXPECT_EQ(1.0, mesh.property(texCoordsPerFace, *f_iter)[1]) << "Texcoords wrong on index 1 with face: " << f_iter->idx();
        EXPECT_EQ(-1.0f, mesh.property(texCoordsPerFace, *f_iter)[2]) << "Texcoords wrong on index 2 with face: " << f_iter->idx();
        EXPECT_EQ(-1.0f, mesh.property(texCoordsPerFace, *f_iter)[3]) << "Texcoords wrong on index 3 with face: " << f_iter->idx();
        EXPECT_EQ(0.0f, mesh.property(texCoordsPerFace, *f_iter)[4]) << "Texcoords wrong on index 4 with face: " << f_iter->idx();
        EXPECT_EQ(0.0f, mesh.property(texCoordsPerFace, *f_iter)[5]) << "Texcoords wrong on index 5 with face: " << f_iter->idx();
        EXPECT_EQ(-0.5f, mesh.property(texCoordsPerFace, *f_iter)[6]) << "Texcoords wrong on index 6 with face: " << f_iter->idx();
        EXPECT_EQ(-0.5f, mesh.property(texCoordsPerFace, *f_iter)[7]) << "Texcoords wrong on index 7 with face: " << f_iter->idx();
      }

    }

    OpenMesh::FPropHandleT< unsigned > faceIndex;
    ASSERT_TRUE(mesh.get_property_handle(faceIndex,"faceIndex")) << "Could not access faceIndex per face";

    EXPECT_EQ(0u,mesh.property(faceIndex,OpenMesh::FaceHandle(0))) << "Wrong index value at FaceHandle 0";
    EXPECT_EQ(1u,mesh.property(faceIndex,OpenMesh::FaceHandle(1))) << "Wrong index value at FaceHandle 1";
    EXPECT_EQ(2u,mesh.property(faceIndex,OpenMesh::FaceHandle(2))) << "Wrong index value at FaceHandle 2";
    EXPECT_EQ(3u,mesh.property(faceIndex,OpenMesh::FaceHandle(3))) << "Wrong index value at FaceHandle 3";
    EXPECT_EQ(4u,mesh.property(faceIndex,OpenMesh::FaceHandle(4))) << "Wrong index value at FaceHandle 4";
    EXPECT_EQ(5u,mesh.property(faceIndex,OpenMesh::FaceHandle(5))) << "Wrong index value at FaceHandle 5";

}

/*
 * Just load a ply with custom properties, binary mode
 */
TEST_F(OpenMeshReadWritePLY, LoadSimplePLYWithCustomPropsBinary) {

    PolyMesh mesh;

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::Custom;
    options += OpenMesh::IO::Options::Binary;

    bool ok = OpenMesh::IO::read_mesh(mesh, "cube-minimal-custom_props-binary.ply", options);

    EXPECT_TRUE(ok) << "Unable to load cube-minimal-custom_props.ply";

    EXPECT_EQ(8u  , mesh.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(12u , mesh.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(6u , mesh.n_faces()) << "The number of loaded faces is not correct!";

    OpenMesh::VPropHandleT<float> qualityProp;
    OpenMesh::VPropHandleT<unsigned int> indexProp;
    ASSERT_TRUE(mesh.get_property_handle(qualityProp,"quality")) << "Could not access quality property";
    ASSERT_TRUE(mesh.get_property_handle(indexProp,"index")) << "Could not access index property";

    //check index property
    for (unsigned i = 0; i < mesh.n_vertices(); ++i)
      EXPECT_EQ(i ,mesh.property(indexProp,OpenMesh::VertexHandle(i))) << "Vertex index at vertex " << i << " is wrong";

    //check quality property
    EXPECT_EQ(1.f,mesh.property(qualityProp,OpenMesh::VertexHandle(0))) << "Wrong quality value at Vertex 0";
    EXPECT_EQ(0.5f,mesh.property(qualityProp,OpenMesh::VertexHandle(1))) << "Wrong quality value at Vertex 1";
    EXPECT_EQ(0.7f,mesh.property(qualityProp,OpenMesh::VertexHandle(2))) << "Wrong quality value at Vertex 2";
    EXPECT_EQ(1.f,mesh.property(qualityProp,OpenMesh::VertexHandle(3))) << "Wrong quality value at Vertex 3";
    EXPECT_EQ(0.1f,mesh.property(qualityProp,OpenMesh::VertexHandle(4))) << "Wrong quality value at Vertex 4";
    EXPECT_EQ(0.f,mesh.property(qualityProp,OpenMesh::VertexHandle(5))) << "Wrong quality value at Vertex 5";
    EXPECT_EQ(2.f,mesh.property(qualityProp,OpenMesh::VertexHandle(6))) << "Wrong quality value at Vertex 6";
    EXPECT_EQ(5.f,mesh.property(qualityProp,OpenMesh::VertexHandle(7))) << "Wrong quality value at Vertex 7";

    //check for custom list properties

    OpenMesh::VPropHandleT< std::vector<int> > testValues;
    ASSERT_TRUE(mesh.get_property_handle(testValues,"test_values")) << "Could not access texcoords per face";

    EXPECT_EQ(2u,mesh.property(testValues,OpenMesh::VertexHandle(0)).size()) << "Wrong verctor size";

    EXPECT_EQ(1,mesh.property(testValues,OpenMesh::VertexHandle(0))[0]) << "Wrong list value at Vertex 0";
    EXPECT_EQ(4,mesh.property(testValues,OpenMesh::VertexHandle(1))[1]) << "Wrong list value at Vertex 1";
    EXPECT_EQ(5,mesh.property(testValues,OpenMesh::VertexHandle(2))[0]) << "Wrong list value at Vertex 2";
    EXPECT_EQ(8,mesh.property(testValues,OpenMesh::VertexHandle(3))[1]) << "Wrong list value at Vertex 3";
    EXPECT_EQ(9,mesh.property(testValues,OpenMesh::VertexHandle(4))[0]) << "Wrong list value at Vertex 4";
    EXPECT_EQ(12,mesh.property(testValues,OpenMesh::VertexHandle(5))[1]) << "Wrong list value at Vertex 5";
    EXPECT_EQ(13,mesh.property(testValues,OpenMesh::VertexHandle(6))[0]) << "Wrong list value at Vertex 6";
    EXPECT_EQ(16,mesh.property(testValues,OpenMesh::VertexHandle(7))[1]) << "Wrong list value at Vertex 7";

    OpenMesh::FPropHandleT< std::vector<float> > texCoordsPerFace;
    ASSERT_TRUE(mesh.get_property_handle(texCoordsPerFace,"texcoords")) << "Could not access texcoords per face";

    for (Mesh::FaceIter f_iter = mesh.faces_begin(); f_iter != mesh.faces_end(); ++f_iter)
    {
      EXPECT_EQ(8u, mesh.property(texCoordsPerFace, *f_iter).size()) << "Texcoords per face container has wrong size on face: " << f_iter->idx();
      if (!mesh.property(texCoordsPerFace, *f_iter).empty())
      {
        EXPECT_EQ(1.0, mesh.property(texCoordsPerFace, *f_iter)[0]) << "Texcoords wrong on index 0 with face: " << f_iter->idx();
        EXPECT_EQ(1.0, mesh.property(texCoordsPerFace, *f_iter)[1]) << "Texcoords wrong on index 1 with face: " << f_iter->idx();
        EXPECT_EQ(-1.0f, mesh.property(texCoordsPerFace, *f_iter)[2]) << "Texcoords wrong on index 2 with face: " << f_iter->idx();
        EXPECT_EQ(-1.0f, mesh.property(texCoordsPerFace, *f_iter)[3]) << "Texcoords wrong on index 3 with face: " << f_iter->idx();
        EXPECT_EQ(0.0f, mesh.property(texCoordsPerFace, *f_iter)[4]) << "Texcoords wrong on index 4 with face: " << f_iter->idx();
        EXPECT_EQ(0.0f, mesh.property(texCoordsPerFace, *f_iter)[5]) << "Texcoords wrong on index 5 with face: " << f_iter->idx();
        EXPECT_EQ(-0.5f, mesh.property(texCoordsPerFace, *f_iter)[6]) << "Texcoords wrong on index 6 with face: " << f_iter->idx();
        EXPECT_EQ(-0.5f, mesh.property(texCoordsPerFace, *f_iter)[7]) << "Texcoords wrong on index 7 with face: " << f_iter->idx();
      }

    }

    OpenMesh::FPropHandleT< unsigned > faceIndex;
    ASSERT_TRUE(mesh.get_property_handle(faceIndex,"faceIndex")) << "Could not access faceIndex per face";

    EXPECT_EQ(0u,mesh.property(faceIndex,OpenMesh::FaceHandle(0))) << "Wrong index value at FaceHandle 0";
    EXPECT_EQ(1u,mesh.property(faceIndex,OpenMesh::FaceHandle(1))) << "Wrong index value at FaceHandle 1";
    EXPECT_EQ(2u,mesh.property(faceIndex,OpenMesh::FaceHandle(2))) << "Wrong index value at FaceHandle 2";
    EXPECT_EQ(3u,mesh.property(faceIndex,OpenMesh::FaceHandle(3))) << "Wrong index value at FaceHandle 3";
    EXPECT_EQ(4u,mesh.property(faceIndex,OpenMesh::FaceHandle(4))) << "Wrong index value at FaceHandle 4";
    EXPECT_EQ(5u,mesh.property(faceIndex,OpenMesh::FaceHandle(5))) << "Wrong index value at FaceHandle 5";

}

TEST_F(OpenMeshReadWritePLY, WriteReadSimplePLYWithCustomProps) {

    PolyMesh mesh;

    OpenMesh::IO::Options options;
    bool ok = OpenMesh::IO::read_mesh(mesh, "cube-minimal.ply", options);


    OpenMesh::VPropHandleT<unsigned short> indexProp;
    OpenMesh::VPropHandleT<unsigned int> nonPersistant;
    OpenMesh::VPropHandleT<double> qualityProp;
    OpenMesh::FPropHandleT<signed int> faceProp;
    OpenMesh::VPropHandleT<int> removedProp;

    const std::string indexPropName = "mySuperIndexProperty";
    const std::string qualityPropName = "quality";
    const std::string facePropName = "anotherPropForFaces";
    const std::string nonPersistantName = "nonPersistant";
    const std::string removedPropName = "willBeRemoved";

    mesh.add_property(indexProp,indexPropName);
    mesh.add_property(qualityProp,qualityPropName);
    mesh.add_property(removedProp, removedPropName);
    mesh.add_property(faceProp,facePropName);
    mesh.add_property(nonPersistant,nonPersistantName);

    mesh.property(indexProp).set_persistent(true);
    mesh.property(qualityProp).set_persistent(true);
    mesh.property(faceProp).set_persistent(true);
    mesh.remove_property(removedProp);

    signed char i=0;
    for (Mesh::VertexIter v_iter = mesh.vertices_begin(); v_iter != mesh.vertices_end(); ++v_iter, ++i)
    {
      mesh.property(indexProp, *v_iter) = i;
      mesh.property(qualityProp, *v_iter) = 3.5*i;
    }

    i = 0;
    for (Mesh::FaceIter f_iter = mesh.faces_begin(); f_iter != mesh.faces_end(); ++f_iter, ++i)
    {
      mesh.property(faceProp, *f_iter) = -i;
    }

    const char* outFilename = "cube-minimal-customprops_openmeshOutputTestfile.ply";
    ok = OpenMesh::IO::write_mesh(mesh, outFilename);

    ASSERT_TRUE(ok);

    PolyMesh loadedMesh;

    EXPECT_FALSE(loadedMesh.get_property_handle(indexProp,indexPropName)) << "Could access to property which was deleted";

    options += OpenMesh::IO::Options::Custom;
    ok = OpenMesh::IO::read_mesh(loadedMesh, outFilename, options);

    ASSERT_TRUE(ok);


    ASSERT_TRUE(loadedMesh.get_property_handle(indexProp,indexPropName)) << "Could not access index property";
    ASSERT_TRUE(loadedMesh.get_property_handle(qualityProp,qualityPropName)) << "Could not access quality property";
    ASSERT_TRUE(loadedMesh.get_property_handle(faceProp,facePropName)) << "Could not access face property";
    EXPECT_FALSE(loadedMesh.get_property_handle(nonPersistant,nonPersistantName)) << "Could access non persistant property";

    i=0;
    for (Mesh::VertexIter v_iter = loadedMesh.vertices_begin(); v_iter != loadedMesh.vertices_end(); ++v_iter, ++i)
    {
      EXPECT_EQ(loadedMesh.property(indexProp, *v_iter), static_cast<unsigned>(i));
      EXPECT_EQ(loadedMesh.property(qualityProp, *v_iter),3.5*i);
    }

    i = 0;
    for (Mesh::FaceIter f_iter = loadedMesh.faces_begin(); f_iter != loadedMesh.faces_end(); ++f_iter, ++i)
    {
      EXPECT_EQ(loadedMesh.property(faceProp, *f_iter),-i);
    }


    remove(outFilename);

}

TEST_F(OpenMeshReadWritePLY, WriteReadBinaryPLYWithCustomProps) {

    PolyMesh mesh;

    OpenMesh::IO::Options options;
    bool ok = OpenMesh::IO::read_mesh(mesh, "cube-minimal.ply", options);


    OpenMesh::VPropHandleT<unsigned short> indexProp;
    OpenMesh::VPropHandleT<unsigned int> nonPersistant;
    OpenMesh::VPropHandleT<double> qualityProp;
    OpenMesh::FPropHandleT<signed int> faceProp;
    OpenMesh::VPropHandleT<int> removedProp;

    const std::string indexPropName = "mySuperIndexProperty";
    const std::string qualityPropName = "quality";
    const std::string facePropName = "anotherPropForFaces";
    const std::string nonPersistantName = "nonPersistant";
    const std::string removedPropName = "willBeRemoved";

    mesh.add_property(indexProp,indexPropName);
    mesh.add_property(qualityProp,qualityPropName);
    mesh.add_property(removedProp, removedPropName);
    mesh.add_property(faceProp,facePropName);
    mesh.add_property(nonPersistant,nonPersistantName);

    mesh.property(indexProp).set_persistent(true);
    mesh.property(qualityProp).set_persistent(true);
    mesh.property(faceProp).set_persistent(true);
    mesh.remove_property(removedProp);

    signed char i=0;
    for (Mesh::VertexIter v_iter = mesh.vertices_begin(); v_iter != mesh.vertices_end(); ++v_iter, ++i)
    {
      mesh.property(indexProp, *v_iter) = i;
      mesh.property(qualityProp, *v_iter) = 3.5*i;
    }

    i = 0;
    for (Mesh::FaceIter f_iter = mesh.faces_begin(); f_iter != mesh.faces_end(); ++f_iter, ++i)
    {
      mesh.property(faceProp, *f_iter) = -i;
    }

    const char* outFilename = "cube-minimal-customprops_openmeshOutputTestfileBinary.ply";
    options += OpenMesh::IO::Options::Binary;
    ok = OpenMesh::IO::write_mesh(mesh, outFilename, options);

    ASSERT_TRUE(ok);

    PolyMesh loadedMesh;

    EXPECT_FALSE(loadedMesh.get_property_handle(indexProp,indexPropName)) << "Could access to property which was deleted";

    options.clear();
    options += OpenMesh::IO::Options::Custom;
    options += OpenMesh::IO::Options::Binary;
    ok = OpenMesh::IO::read_mesh(loadedMesh, outFilename, options);

    ASSERT_TRUE(ok);


    ASSERT_TRUE(loadedMesh.get_property_handle(indexProp,indexPropName)) << "Could not access index property";
    ASSERT_TRUE(loadedMesh.get_property_handle(qualityProp,qualityPropName)) << "Could not access quality property";
    ASSERT_TRUE(loadedMesh.get_property_handle(faceProp,facePropName)) << "Could not access face property";
    EXPECT_FALSE(loadedMesh.get_property_handle(nonPersistant,nonPersistantName)) << "Could access non persistant property";

    i=0;
    for (Mesh::VertexIter v_iter = loadedMesh.vertices_begin(); v_iter != loadedMesh.vertices_end(); ++v_iter, ++i)
    {
      EXPECT_EQ(loadedMesh.property(indexProp, *v_iter), static_cast<unsigned>(i));
      EXPECT_EQ(loadedMesh.property(qualityProp, *v_iter),3.5*i);
    }

    i = 0;
    for (Mesh::FaceIter f_iter = loadedMesh.faces_begin(); f_iter != loadedMesh.faces_end(); ++f_iter, ++i)
    {
      EXPECT_EQ(loadedMesh.property(faceProp, *f_iter),-i);
    }


    //remove(outFilename);

}

/*
* Just load a ply with extra elements
*/
TEST_F(OpenMeshReadWritePLY, LoadSimplePLYWithExtraElements) {

	mesh_.clear();

	bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-extra-elements.ply");

	EXPECT_TRUE(ok) << "Unable to load cube-minimal-extra-elements.ply";

	EXPECT_EQ(8u, mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
	EXPECT_EQ(18u, mesh_.n_edges()) << "The number of loaded edges is not correct!";
	EXPECT_EQ(12u, mesh_.n_faces()) << "The number of loaded faces is not correct!";

}

/*
* Just load a binary ply with extra elements
*/
TEST_F(OpenMeshReadWritePLY, LoadSimpleBinaryPLYWithExtraElements) {

	mesh_.clear();

	OpenMesh::IO::Options options = OpenMesh::IO::Options::Binary;

	bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-extra-elements-binary.ply", options);

	EXPECT_TRUE(ok) << "Unable to load cube-minimal-extra-elements-binary.ply";

	EXPECT_EQ(8u, mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
	EXPECT_EQ(18u, mesh_.n_edges()) << "The number of loaded edges is not correct!";
	EXPECT_EQ(12u, mesh_.n_faces()) << "The number of loaded faces is not correct!";

}

/*
* Ignore a file that does not contain vertices and faces
*/
TEST_F(OpenMeshReadWritePLY, IgnoreNonMeshPlyFile) {

    mesh_.clear();

    std::stringstream data;
    data << "ply" << "\n";
    data << "format binary_little_endian 1.0" << "\n";
    data << "comment Image data" << "\n";
    data << "element image 0" << "\n";
    data << "property list uint16 uint16 row" << "\n";
    data << "end_header" << "\n";

    OpenMesh::IO::Options options = OpenMesh::IO::Options::Binary;

    bool ok = OpenMesh::IO::read_mesh(mesh_, data, ".ply", options);

    EXPECT_TRUE(ok) << "This empty file should be readable without an error!";

    EXPECT_EQ(0u, mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(0u, mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(0u, mesh_.n_faces()) << "The number of loaded faces is not correct!";
}


/*
* Ignore a file that does not contain vertices and faces
*/
TEST_F(OpenMeshReadWritePLY, FailOnUnknownPropertyTypeForLists) {

    mesh_.clear();

    std::stringstream data;
    data << "ply" << "\n";
    data << "format binary_little_endian 1.0" << "\n";
    data << "comment Image data" << "\n";
    data << "element image 0" << "\n";
    data << "property list blibb blubb row" << "\n";
    data << "end_header" << "\n";

    OpenMesh::IO::Options options = OpenMesh::IO::Options::Binary;

    bool ok = OpenMesh::IO::read_mesh(mesh_, data, ".ply", options);

    EXPECT_FALSE(ok) << "This file should fail to read!";

    EXPECT_EQ(0u, mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(0u, mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(0u, mesh_.n_faces()) << "The number of loaded faces is not correct!";
}

/*
 * Load an ASCII PLY file of a cube with float RGB face colors
 */
TEST_F(OpenMeshReadWritePLY, LoadSimplePLYWithFaceColors) {

    mesh_.clear();

    mesh_.request_face_colors();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::FaceColor;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-faceColors.ply",options);

    EXPECT_TRUE(ok) << "Unable to load cube-minimal-faceColors.ply";

    EXPECT_EQ(8u , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u, mesh_.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u, mesh_.n_faces())    << "The number of loaded faces is not correct!";

#ifdef TEST_DOUBLE_TRAITS
    EXPECT_FLOAT_EQ(107/255.0, mesh_.color(mesh_.face_handle(0))[0] ) << "Wrong face color at face 0";
    EXPECT_FLOAT_EQ(117/255.0, mesh_.color(mesh_.face_handle(0))[1] ) << "Wrong face color at face 0";
    EXPECT_FLOAT_EQ(177/255.0, mesh_.color(mesh_.face_handle(0))[2] ) << "Wrong face color at face 0";

    EXPECT_FLOAT_EQ(107/255.0, mesh_.color(mesh_.face_handle(3))[0] ) << "Wrong face color at face 3";
    EXPECT_FLOAT_EQ(255/255.0, mesh_.color(mesh_.face_handle(3))[1] ) << "Wrong face color at face 3";
    EXPECT_FLOAT_EQ(135/255.0, mesh_.color(mesh_.face_handle(3))[2] ) << "Wrong face color at face 3";

    EXPECT_FLOAT_EQ(163/255.0, mesh_.color(mesh_.face_handle(4))[0] ) << "Wrong face color at face 4";
    EXPECT_FLOAT_EQ(107/255.0, mesh_.color(mesh_.face_handle(4))[1] ) << "Wrong face color at face 4";
    EXPECT_FLOAT_EQ(177/255.0, mesh_.color(mesh_.face_handle(4))[2] ) << "Wrong face color at face 4";

    EXPECT_FLOAT_EQ(255/255.0, mesh_.color(mesh_.face_handle(7))[0] ) << "Wrong face color at face 7";
    EXPECT_FLOAT_EQ(140/255.0, mesh_.color(mesh_.face_handle(7))[1] ) << "Wrong face color at face 7";
    EXPECT_FLOAT_EQ(107/255.0, mesh_.color(mesh_.face_handle(7))[2] ) << "Wrong face color at face 7";
#else
    EXPECT_EQ(107, mesh_.color(mesh_.face_handle(0))[0] ) << "Wrong face color at face 0";
    EXPECT_EQ(117, mesh_.color(mesh_.face_handle(0))[1] ) << "Wrong face color at face 0";
    EXPECT_EQ(177, mesh_.color(mesh_.face_handle(0))[2] ) << "Wrong face color at face 0";

    EXPECT_EQ(107, mesh_.color(mesh_.face_handle(3))[0] ) << "Wrong face color at face 3";
    EXPECT_EQ(255, mesh_.color(mesh_.face_handle(3))[1] ) << "Wrong face color at face 3";
    EXPECT_EQ(135, mesh_.color(mesh_.face_handle(3))[2] ) << "Wrong face color at face 3";

    EXPECT_EQ(163, mesh_.color(mesh_.face_handle(4))[0] ) << "Wrong face color at face 4";
    EXPECT_EQ(107, mesh_.color(mesh_.face_handle(4))[1] ) << "Wrong face color at face 4";
    EXPECT_EQ(177, mesh_.color(mesh_.face_handle(4))[2] ) << "Wrong face color at face 4";

    EXPECT_EQ(255, mesh_.color(mesh_.face_handle(7))[0] ) << "Wrong face color at face 7";
    EXPECT_EQ(140, mesh_.color(mesh_.face_handle(7))[1] ) << "Wrong face color at face 7";
    EXPECT_EQ(107, mesh_.color(mesh_.face_handle(7))[2] ) << "Wrong face color at face 7";
#endif

    EXPECT_FALSE(options.vertex_has_normal()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.vertex_has_texcoord()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.vertex_has_color()) << "Wrong user options are returned!";
    EXPECT_TRUE(options.face_has_color()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.color_has_alpha()) << "Wrong user options are returned!";
    EXPECT_FALSE(options.is_binary()) << "Wrong user options are returned!";

    mesh_.release_face_colors();
}

/*
 * Write and read PLY files with face colors in various formats
 */
TEST_F(OpenMeshReadWritePLY, WriteAndReadPLYWithFaceColors) {
    struct Format {
        Format(const char* outFileName, OpenMesh::IO::Options options) :
            _outFileName(outFileName),
            _options(options)
        {}
        const char*  _outFileName;
        const OpenMesh::IO::Options  _options;
    }
    formats[] =
    {
        Format("cube-minimal-faceColors_ascii_uchar.ply",
               OpenMesh::IO::Options::Default),
        Format("cube-minimal-faceColors_ascii_float.ply",
               OpenMesh::IO::Options::ColorFloat),
        Format("cube-minimal-faceColors_binary_uchar.ply",
               OpenMesh::IO::Options::Binary),
        Format("cube-minimal-faceColors_binary_float.ply",
               OpenMesh::IO::Options::Binary | OpenMesh::IO::Options::ColorFloat),
        // Test writing/reading alpha values (all default 1.0/255), but the test mesh
        // Color type has no alpha channel so there's nothing to test below
        Format("cube-minimal-faceColors_alpha_ascii_uchar.ply",
               OpenMesh::IO::Options::ColorAlpha),
        Format("cube-minimal-faceColors_alpha_ascii_float.ply",
               OpenMesh::IO::Options::ColorFloat | OpenMesh::IO::Options::ColorAlpha),
        Format("cube-minimal-faceColors_alpha_binary_uchar.ply",
               OpenMesh::IO::Options::Binary | OpenMesh::IO::Options::ColorAlpha),
        Format("cube-minimal-faceColors_alpha_binary_float.ply",
               OpenMesh::IO::Options::Binary | OpenMesh::IO::Options::ColorFloat | OpenMesh::IO::Options::ColorAlpha),
    };

    for (size_t i = 0; i < sizeof(formats) / sizeof(Format); ++i)
    {
        const char*  outFileName = formats[i]._outFileName;

        mesh_.clear();

        mesh_.request_face_colors();

        OpenMesh::IO::Options options;
        options += OpenMesh::IO::Options::FaceColor;

        bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-faceColors.ply", options);

        EXPECT_TRUE(ok) << "Unable to load cube-minimal-faceColors.ply";

        options = formats[i]._options;
        options += OpenMesh::IO::Options::FaceColor;
        ok = OpenMesh::IO::write_mesh(mesh_, outFileName, options);
        EXPECT_TRUE(ok) << "Unable to write " << outFileName;

        // Reset for reading: let the reader determine binary/float/etc.
        options = OpenMesh::IO::Options::FaceColor;
        mesh_.clear();
        ok = OpenMesh::IO::read_mesh(mesh_, outFileName, options);
        EXPECT_TRUE(ok) << "Unable to load " << outFileName;

        EXPECT_EQ(8u , mesh_.n_vertices()) << "The number of loaded vertices is not correct: " << outFileName;
        EXPECT_EQ(18u, mesh_.n_edges())    << "The number of loaded edges is not correct: " << outFileName;
        EXPECT_EQ(12u, mesh_.n_faces())    << "The number of loaded faces is not correct: " << outFileName;

#ifdef TEST_DOUBLE_TRAITS
        EXPECT_FLOAT_EQ(107/255.0, mesh_.color(mesh_.face_handle(0))[0] ) << "Wrong face color at face 0";
        EXPECT_FLOAT_EQ(117/255.0, mesh_.color(mesh_.face_handle(0))[1] ) << "Wrong face color at face 0";
        EXPECT_FLOAT_EQ(177/255.0, mesh_.color(mesh_.face_handle(0))[2] ) << "Wrong face color at face 0";

        EXPECT_FLOAT_EQ(107/255.0, mesh_.color(mesh_.face_handle(3))[0] ) << "Wrong face color at face 3";
        EXPECT_FLOAT_EQ(255/255.0, mesh_.color(mesh_.face_handle(3))[1] ) << "Wrong face color at face 3";
        EXPECT_FLOAT_EQ(135/255.0, mesh_.color(mesh_.face_handle(3))[2] ) << "Wrong face color at face 3";

        EXPECT_FLOAT_EQ(163/255.0, mesh_.color(mesh_.face_handle(4))[0] ) << "Wrong face color at face 4";
        EXPECT_FLOAT_EQ(107/255.0, mesh_.color(mesh_.face_handle(4))[1] ) << "Wrong face color at face 4";
        EXPECT_FLOAT_EQ(177/255.0, mesh_.color(mesh_.face_handle(4))[2] ) << "Wrong face color at face 4";

        EXPECT_FLOAT_EQ(255/255.0, mesh_.color(mesh_.face_handle(7))[0] ) << "Wrong face color at face 7";
        EXPECT_FLOAT_EQ(140/255.0, mesh_.color(mesh_.face_handle(7))[1] ) << "Wrong face color at face 7";
        EXPECT_FLOAT_EQ(107/255.0, mesh_.color(mesh_.face_handle(7))[2] ) << "Wrong face color at face 7";
#else
        EXPECT_EQ(107, mesh_.color(mesh_.face_handle(0))[0] ) << "Wrong face color at face 0";
        EXPECT_EQ(117, mesh_.color(mesh_.face_handle(0))[1] ) << "Wrong face color at face 0";
        EXPECT_EQ(177, mesh_.color(mesh_.face_handle(0))[2] ) << "Wrong face color at face 0";

        EXPECT_EQ(107, mesh_.color(mesh_.face_handle(3))[0] ) << "Wrong face color at face 3";
        EXPECT_EQ(255, mesh_.color(mesh_.face_handle(3))[1] ) << "Wrong face color at face 3";
        EXPECT_EQ(135, mesh_.color(mesh_.face_handle(3))[2] ) << "Wrong face color at face 3";

        EXPECT_EQ(163, mesh_.color(mesh_.face_handle(4))[0] ) << "Wrong face color at face 4";
        EXPECT_EQ(107, mesh_.color(mesh_.face_handle(4))[1] ) << "Wrong face color at face 4";
        EXPECT_EQ(177, mesh_.color(mesh_.face_handle(4))[2] ) << "Wrong face color at face 4";

        EXPECT_EQ(255, mesh_.color(mesh_.face_handle(7))[0] ) << "Wrong face color at face 7";
        EXPECT_EQ(140, mesh_.color(mesh_.face_handle(7))[1] ) << "Wrong face color at face 7";
        EXPECT_EQ(107, mesh_.color(mesh_.face_handle(7))[2] ) << "Wrong face color at face 7";
#endif

        EXPECT_FALSE(options.vertex_has_normal()) << "Wrong user options are returned: " << outFileName;
        EXPECT_FALSE(options.vertex_has_texcoord()) << "Wrong user options are returned: " << outFileName;
        EXPECT_FALSE(options.vertex_has_color()) << "Wrong user options are returned: " << outFileName;
        EXPECT_TRUE(options.face_has_color()) << "Wrong user options are returned: " << outFileName;
        EXPECT_EQ(formats[i]._options.color_is_float(), options.color_is_float()) <<
            "Wrong user options are returned: " << outFileName;
        EXPECT_EQ(formats[i]._options.is_binary(), options.is_binary()) <<
            "Wrong user options are returned: " << outFileName;

        mesh_.release_face_colors();
    }
}

}

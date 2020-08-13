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

    EXPECT_EQ(255,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(255,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(255,   mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";

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

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(255,   mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(255,   mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";

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

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(255,   mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(255,   mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";

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

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(255,   mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(255,   mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";

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

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(255,   mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(255,   mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";

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
}

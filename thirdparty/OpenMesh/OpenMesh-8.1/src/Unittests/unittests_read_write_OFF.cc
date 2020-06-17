#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <cstdio>


namespace {

class OpenMeshReadWriteOFF : public OpenMeshBase {

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
 * Just load a simple mesh file in obj format and count whether
 * the right number of entities has been loaded.
 */
TEST_F(OpenMeshReadWriteOFF, LoadSimpleOFFFile) {

    mesh_.clear();

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.off");

    EXPECT_TRUE(ok);

    EXPECT_EQ(7526u , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(22572u, mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(15048u, mesh_.n_faces()) << "The number of loaded faces is not correct!";
}


TEST_F(OpenMeshReadWriteOFF, WriteAndReadVertexColorsToAndFromOFFFile) {

    mesh_.clear();

    mesh_.request_vertex_colors();

    mesh_.add_vertex( Mesh::Point(0,0,1) );
    mesh_.add_vertex( Mesh::Point(0,1,0) );
    mesh_.add_vertex( Mesh::Point(0,1,1) );
    mesh_.add_vertex( Mesh::Point(1,0,1) );

#ifdef TEST_DOUBLE_TRAITS
    // using the default color type Vec4f from DefaultTraitsDouble in Traits.hh
    Mesh::Color testColor(255/255.0, 128/255.0, 64/255.0, 1.0);
#else
    // using the default color type Vec3uc from DefaultTraits in Traits.hh
    Mesh::Color testColor(255, 128, 64);
#endif

    // setting colors (different from black)
    for (Mesh::VertexIter vit = mesh_.vertices_begin(), vitend = mesh_.vertices_end(); vit != vitend; ++vit)
      mesh_.set_color(*vit, testColor);

    // check if the colors are correctly setted
    int count = 0;
    for (Mesh::VertexIter vit = mesh_.vertices_begin(), vitend = mesh_.vertices_end(); vit != vitend; ++vit) {
      Mesh::Color color = mesh_.color(*vit);
      bool wrong_color = false;
      for (size_t i = 0; i < color.size(); ++i)
        wrong_color = wrong_color || (color[i] != testColor[i]);
      if (wrong_color)
        ++ count;
    }

    EXPECT_EQ(0, count) << "Vertices have the wrong color!";

    // write the mesh_
    OpenMesh::IO::Options opt(OpenMesh::IO::Options::VertexColor);
    OpenMesh::IO::write_mesh(mesh_, "temp.off", opt);
    OpenMesh::IO::read_mesh(mesh_, "temp.off", opt);
    remove("temp.off");

    // check if vertices still have the same color
    count = 0;
    for (Mesh::VertexIter vit = mesh_.vertices_begin(), vitend = mesh_.vertices_end(); vit != vitend; ++vit) {
      Mesh::Color color = mesh_.color(*vit);
      bool wrong_color = false;
      for (size_t i = 0; i < color.size(); ++i)
        wrong_color = wrong_color || (color[i] != testColor[i]);
      if (wrong_color)
        ++ count;
    }

    EXPECT_EQ(0, count) << "Vertices should have the same color after writing and reading the OFF file!";

    mesh_.release_vertex_colors();
}

TEST_F(OpenMeshReadWriteOFF, WriteAndReadFloatVertexColorsToAndFromOFFFile) {

    mesh_.clear();

    mesh_.request_vertex_colors();

    OpenMesh::IO::Options opt(OpenMesh::IO::Options::VertexColor);

    bool ok = OpenMesh::IO::read_mesh(mesh_, "meshlab.ply", opt);

    EXPECT_TRUE(ok) << "meshlab.ply could not be read!";

    opt.clear();
    opt += OpenMesh::IO::Options::VertexColor;
    opt += OpenMesh::IO::Options::ColorFloat;

    // write the mesh_
    ok = OpenMesh::IO::write_mesh(mesh_, "cube_floating.off", opt);
    EXPECT_TRUE(ok) << "cube_floating.off could not be written!";
    mesh_.clear();
    ok = OpenMesh::IO::read_mesh(mesh_, "cube_floating.off", opt);
    EXPECT_TRUE(ok) << "cube_floating.off could not be read!";

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

    EXPECT_FALSE(opt.vertex_has_normal()) << "Wrong user opt are returned!";
    EXPECT_FALSE(opt.vertex_has_texcoord()) << "Wrong user opt are returned!";
    EXPECT_TRUE(opt.vertex_has_color()) << "Wrong user opt are returned!";
    EXPECT_TRUE(opt.color_is_float()) << "Wrong user opt are returned!";

    mesh_.release_vertex_colors();
}

TEST_F(OpenMeshReadWriteOFF, WriteAndReadBinaryFloatVertexColorsToAndFromOFFFile) {

    mesh_.clear();

    mesh_.request_vertex_colors();

    OpenMesh::IO::Options opt(OpenMesh::IO::Options::VertexColor);

    bool ok = OpenMesh::IO::read_mesh(mesh_, "meshlab.ply", opt);

    EXPECT_TRUE(ok) << "meshlab.ply could not be read!";

    opt.clear();
    opt += OpenMesh::IO::Options::VertexColor;
    opt += OpenMesh::IO::Options::Binary;
    opt += OpenMesh::IO::Options::ColorFloat;

    // write the mesh_
    ok = OpenMesh::IO::write_mesh(mesh_, "cube_floating_binary.off", opt);
    EXPECT_TRUE(ok) << "cube_floating_binary.off could not be written!";
    mesh_.clear();
    opt.clear();
    opt += OpenMesh::IO::Options::VertexColor;
    opt += OpenMesh::IO::Options::Binary;
    opt += OpenMesh::IO::Options::ColorFloat;
    ok = OpenMesh::IO::read_mesh(mesh_, "cube_floating_binary.off", opt);
    EXPECT_TRUE(ok) << "cube_floating_binary.off could not be read!";

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

    EXPECT_FALSE(opt.vertex_has_normal()) << "Wrong user opt are returned!";
    EXPECT_FALSE(opt.vertex_has_texcoord()) << "Wrong user opt are returned!";
    EXPECT_FALSE(opt.face_has_color()) << "Wrong user opt are returned!";
    EXPECT_TRUE(opt.vertex_has_color()) << "Wrong user opt are returned!";
    EXPECT_TRUE(opt.color_is_float()) << "Wrong user opt are returned!";
    EXPECT_TRUE(opt.is_binary()) << "Wrong user opt are returned!";

    mesh_.release_vertex_colors();
}
}

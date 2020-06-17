
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>
#include <cstdio>


namespace {

class OpenMeshReadWriteOBJ : public OpenMeshBase {

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
 * Just load a obj file of a cube
 */
TEST_F(OpenMeshReadWriteOBJ, LoadSimpleOBJ) {

    mesh_.clear();

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal.obj");

    EXPECT_TRUE(ok) << "Unable to load cube-minimal.obj";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces()) << "The number of loaded faces is not correct!";
}

/*
 * Just load a obj file of a cube with degenerated faces
 */
TEST_F(OpenMeshReadWriteOBJ, LoadDegeneratedOBJ) {

    mesh_.clear();

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-degenerated.obj");

    EXPECT_TRUE(ok) << "Unable to load cube-minimal-degenerated.obj";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces()) << "The number of loaded faces is not correct!";
}

/*
 * Just load a obj file of a cube and checks the halfedge and vertex normals
 */
TEST_F(OpenMeshReadWriteOBJ, LoadSimpleOBJCheckHalfEdgeAndVertexNormals) {

  mesh_.clear();

  mesh_.request_halfedge_normals();
  mesh_.request_vertex_normals();

  OpenMesh::IO::Options options;
  options += OpenMesh::IO::Options::VertexNormal;

  std::string file_name = "cube-minimal.obj";

  bool ok = OpenMesh::IO::read_mesh(mesh_, file_name,options);

  EXPECT_TRUE(ok) << file_name;

  EXPECT_EQ(8u  ,   mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
  EXPECT_EQ(18u ,  mesh_.n_edges()) << "The number of loaded edges is not correct!";
  EXPECT_EQ(12u ,  mesh_.n_faces()) << "The number of loaded faces is not correct!";
  EXPECT_EQ(36u , mesh_.n_halfedges()) << "The number of loaded halfedges is not correct!";

  ///////////////////////////////////////////////
  //check vertex normals
  EXPECT_EQ(0,   mesh_.normal(mesh_.vertex_handle(0))[0] ) << "Wrong vertex normal at vertex 0 component 0";
  EXPECT_EQ(-1,   mesh_.normal(mesh_.vertex_handle(0))[1] ) << "Wrong vertex normal at vertex 0 component 1";
  EXPECT_EQ(0,   mesh_.normal(mesh_.vertex_handle(0))[2] ) << "Wrong vertex normal at vertex 0 component 2";

  EXPECT_EQ(0,   mesh_.normal(mesh_.vertex_handle(3))[0] ) << "Wrong vertex normal at vertex 3 component 0";
  EXPECT_EQ(0, mesh_.normal(mesh_.vertex_handle(3))[1] ) << "Wrong vertex normal at vertex 3 component 1";
  EXPECT_EQ(1, mesh_.normal(mesh_.vertex_handle(3))[2] ) << "Wrong vertex normal at vertex 3 component 2";

  EXPECT_EQ(0, mesh_.normal(mesh_.vertex_handle(4))[0] ) << "Wrong vertex normal at vertex 4 component 0";
  EXPECT_EQ(-1,   mesh_.normal(mesh_.vertex_handle(4))[1] ) << "Wrong vertex normal at vertex 4 component 1";
  EXPECT_EQ(0,   mesh_.normal(mesh_.vertex_handle(4))[2] ) << "Wrong vertex normal at vertex 4 component 2";

  EXPECT_EQ(0, mesh_.normal(mesh_.vertex_handle(7))[0] ) << "Wrong vertex normal at vertex 7 component 0";
  EXPECT_EQ(0, mesh_.normal(mesh_.vertex_handle(7))[1] ) << "Wrong vertex normal at vertex 7 component 1";
  EXPECT_EQ(1, mesh_.normal(mesh_.vertex_handle(7))[2] ) << "Wrong vertex normal at vertex 7 component 2";

  ///////////////////////////////////////////////
  //check halfedge normals
  EXPECT_EQ(0,   mesh_.normal(mesh_.halfedge_handle(0))[0] ) << "Wrong halfedge normal at halfedge 0 component 0";
  EXPECT_EQ(0,   mesh_.normal(mesh_.halfedge_handle(0))[1] ) << "Wrong halfedge normal at halfedge 0 component 1";
  EXPECT_EQ(-1,   mesh_.normal(mesh_.halfedge_handle(0))[2] ) << "Wrong halfedge normal at halfedge 0 component 2";

  EXPECT_EQ(-1,   mesh_.normal(mesh_.halfedge_handle(10))[0] ) << "Wrong halfedge normal at halfedge 10 component 0";
  EXPECT_EQ(0, mesh_.normal(mesh_.halfedge_handle(10))[1] ) << "Wrong halfedge normal at halfedge 10 component 1";
  EXPECT_EQ(0, mesh_.normal(mesh_.halfedge_handle(10))[2] ) << "Wrong halfedge normal at halfedge 10 component 2";

  EXPECT_EQ(0, mesh_.normal(mesh_.halfedge_handle(19))[0] ) << "Wrong halfedge normal at halfedge 19 component 0";
  EXPECT_EQ(1,   mesh_.normal(mesh_.halfedge_handle(19))[1] ) << "Wrong halfedge normal at halfedge 19 component 1";
  EXPECT_EQ(0,   mesh_.normal(mesh_.halfedge_handle(19))[2] ) << "Wrong halfedge normal at halfedge 19 component 2";

  EXPECT_EQ(1, mesh_.normal(mesh_.halfedge_handle(24))[0] ) << "Wrong halfedge normal at halfedge 24 component 0";
  EXPECT_EQ(0,   mesh_.normal(mesh_.halfedge_handle(24))[1] ) << "Wrong halfedge normal at halfedge 24 component 1";
  EXPECT_EQ(0,   mesh_.normal(mesh_.halfedge_handle(24))[2] ) << "Wrong halfedge normal at halfedge 24 component 2";

  EXPECT_EQ(0, mesh_.normal(mesh_.halfedge_handle(30))[0] ) << "Wrong halfedge normal at halfedge 30 component 0";
  EXPECT_EQ(-1,   mesh_.normal(mesh_.halfedge_handle(30))[1] ) << "Wrong halfedge normal at halfedge 30 component 1";
  EXPECT_EQ(0,   mesh_.normal(mesh_.halfedge_handle(30))[2] ) << "Wrong halfedge normal at halfedge 30 component 2";

  EXPECT_EQ(0, mesh_.normal(mesh_.halfedge_handle(35))[0] ) << "Wrong halfedge normal at halfedge 35 component 0";
  EXPECT_EQ(0, mesh_.normal(mesh_.halfedge_handle(35))[1] ) << "Wrong halfedge normal at halfedge 35 component 1";
  EXPECT_EQ(1, mesh_.normal(mesh_.halfedge_handle(35))[2] ) << "Wrong halfedge normal at halfedge 35 component 2";

  mesh_.release_vertex_normals();
  mesh_.release_halfedge_normals();

}

/*
 * Just load a obj file and set vertex color option before loading
 */
TEST_F(OpenMeshReadWriteOBJ, LoadSimpleOBJForceVertexColorsAlthoughNotAvailable) {

  mesh_.clear();

  mesh_.request_vertex_colors();

  std::string file_name = "cube-minimal.obj";

  OpenMesh::IO::Options options;
  options += OpenMesh::IO::Options::VertexColor;

  bool ok = OpenMesh::IO::read_mesh(mesh_, file_name,options);

  EXPECT_TRUE(ok) << file_name;

  EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
  EXPECT_EQ(18u , mesh_.n_edges())     << "The number of loaded edges is not correct!";
  EXPECT_EQ(12u , mesh_.n_faces())     << "The number of loaded faces is not correct!";
  EXPECT_EQ(36u , mesh_.n_halfedges())  << "The number of loaded halfedges is not correct!";

}


/*
 * Just load a obj file of a cube and checks the halfedge texCoords
 */
TEST_F(OpenMeshReadWriteOBJ, LoadSimpleOBJCheckTexCoords) {

  mesh_.clear();

  mesh_.request_halfedge_texcoords2D();

  OpenMesh::IO::Options options;
  options += OpenMesh::IO::Options::FaceTexCoord;

  std::string file_name = "cube-minimal-texCoords.obj";

  bool ok = OpenMesh::IO::read_mesh(mesh_, file_name,options);

  EXPECT_TRUE(ok) << file_name;

  EXPECT_EQ(1, mesh_.texcoord2D(mesh_.halfedge_handle(0))[0] ) << "Wrong texCoord at halfedge 0 component 0";
  EXPECT_EQ(1, mesh_.texcoord2D(mesh_.halfedge_handle(0))[1] ) << "Wrong texCoord at halfedge 0 component 1";

  EXPECT_EQ(3, mesh_.texcoord2D(mesh_.halfedge_handle(10))[0] ) << "Wrong texCoord at halfedge 1 component 0";
  EXPECT_EQ(3, mesh_.texcoord2D(mesh_.halfedge_handle(10))[1] ) << "Wrong texCoord at halfedge 1 component 1";

  EXPECT_EQ(6, mesh_.texcoord2D(mesh_.halfedge_handle(19))[0] ) << "Wrong texCoord at halfedge 4 component 0";
  EXPECT_EQ(6, mesh_.texcoord2D(mesh_.halfedge_handle(19))[1] ) << "Wrong texCoord at halfedge 4 component 1";

  EXPECT_EQ(7, mesh_.texcoord2D(mesh_.halfedge_handle(24))[0] ) << "Wrong texCoord at halfedge 7 component 0";
  EXPECT_EQ(7, mesh_.texcoord2D(mesh_.halfedge_handle(24))[1] ) << "Wrong texCoord at halfedge 7 component 1";

  EXPECT_EQ(9, mesh_.texcoord2D(mesh_.halfedge_handle(30))[0] ) << "Wrong texCoord at halfedge 9 component 0";
  EXPECT_EQ(9, mesh_.texcoord2D(mesh_.halfedge_handle(30))[1] ) << "Wrong texCoord at halfedge 9 component 1";

  EXPECT_EQ(12, mesh_.texcoord2D(mesh_.halfedge_handle(35))[0] ) << "Wrong texCoord at halfedge 11 component 0";
  EXPECT_EQ(12, mesh_.texcoord2D(mesh_.halfedge_handle(35))[1] ) << "Wrong texCoord at halfedge 11 component 1";

  mesh_.release_halfedge_texcoords2D();
}

/*
 * Just load and store obj file of a cube and checks the halfedge texCoords
 */
TEST_F(OpenMeshReadWriteOBJ, LoadStoreSimpleOBJCheckTexCoords) {

  mesh_.clear();

  mesh_.request_halfedge_texcoords2D();

  OpenMesh::IO::Options options;
  options += OpenMesh::IO::Options::FaceTexCoord;

  std::string file_name = "cube-minimal-texCoords.obj";

  bool ok = OpenMesh::IO::read_mesh(mesh_, file_name,options);

  EXPECT_TRUE(ok) << file_name;



  options.clear();
  options += OpenMesh::IO::Options::FaceTexCoord;
  bool writeOk = OpenMesh::IO::write_mesh(mesh_,"writeTest.obj",options);
  EXPECT_TRUE(writeOk) << "writeTest.obj";
  mesh_.release_halfedge_texcoords2D();

  Mesh loadedMesh_;
  loadedMesh_.clear();

  loadedMesh_.request_halfedge_texcoords2D();

  options.clear();
  options += OpenMesh::IO::Options::FaceTexCoord;
  bool readOk = OpenMesh::IO::read_mesh(loadedMesh_, "writeTest.obj",options);
  EXPECT_TRUE(readOk) << file_name;
  EXPECT_EQ(1, loadedMesh_.texcoord2D(mesh_.halfedge_handle(0))[0] ) << "Wrong texCoord at halfedge 0 component 0";
  EXPECT_EQ(1, loadedMesh_.texcoord2D(mesh_.halfedge_handle(0))[1] ) << "Wrong texCoord at halfedge 0 component 1";

  EXPECT_EQ(3, loadedMesh_.texcoord2D(mesh_.halfedge_handle(10))[0] ) << "Wrong texCoord at halfedge 1 component 0";
  EXPECT_EQ(3, loadedMesh_.texcoord2D(mesh_.halfedge_handle(10))[1] ) << "Wrong texCoord at halfedge 1 component 1";

  EXPECT_EQ(6, loadedMesh_.texcoord2D(mesh_.halfedge_handle(19))[0] ) << "Wrong texCoord at halfedge 4 component 0";
  EXPECT_EQ(6, loadedMesh_.texcoord2D(mesh_.halfedge_handle(19))[1] ) << "Wrong texCoord at halfedge 4 component 1";

  EXPECT_EQ(7, loadedMesh_.texcoord2D(mesh_.halfedge_handle(24))[0] ) << "Wrong texCoord at halfedge 7 component 0";
  EXPECT_EQ(7, loadedMesh_.texcoord2D(mesh_.halfedge_handle(24))[1] ) << "Wrong texCoord at halfedge 7 component 1";

  EXPECT_EQ(9, loadedMesh_.texcoord2D(mesh_.halfedge_handle(30))[0] ) << "Wrong texCoord at halfedge 9 component 0";
  EXPECT_EQ(9, loadedMesh_.texcoord2D(mesh_.halfedge_handle(30))[1] ) << "Wrong texCoord at halfedge 9 component 1";

  EXPECT_EQ(12, loadedMesh_.texcoord2D(mesh_.halfedge_handle(35))[0] ) << "Wrong texCoord at halfedge 11 component 0";
  EXPECT_EQ(12, loadedMesh_.texcoord2D(mesh_.halfedge_handle(35))[1] ) << "Wrong texCoord at halfedge 11 component 1";

  loadedMesh_.release_halfedge_texcoords2D();
}

/*
 * Just load a obj file of a cube and checks the 3d halfedge texCoords
 */
TEST_F(OpenMeshReadWriteOBJ, LoadSimpleOBJCheckTexCoords3d) {

  mesh_.clear();

  mesh_.request_halfedge_texcoords3D();

  OpenMesh::IO::Options options;
  options += OpenMesh::IO::Options::FaceTexCoord;

  std::string file_name = "cube-minimal-texCoords3d.obj";

  bool ok = OpenMesh::IO::read_mesh(mesh_, file_name,options);

  EXPECT_TRUE(ok) << file_name;

  EXPECT_EQ(1, mesh_.texcoord3D(mesh_.halfedge_handle(0))[0] ) << "Wrong texCoord at halfedge 0 component 0";
  EXPECT_EQ(1, mesh_.texcoord3D(mesh_.halfedge_handle(0))[1] ) << "Wrong texCoord at halfedge 0 component 1";
  EXPECT_EQ(1, mesh_.texcoord3D(mesh_.halfedge_handle(0))[2] ) << "Wrong texCoord at halfedge 0 component 2";

  EXPECT_EQ(3, mesh_.texcoord3D(mesh_.halfedge_handle(10))[0] ) << "Wrong texCoord at halfedge 1 component 0";
  EXPECT_EQ(3, mesh_.texcoord3D(mesh_.halfedge_handle(10))[1] ) << "Wrong texCoord at halfedge 1 component 1";
  EXPECT_EQ(3, mesh_.texcoord3D(mesh_.halfedge_handle(10))[2] ) << "Wrong texCoord at halfedge 1 component 2";

  EXPECT_EQ(6, mesh_.texcoord3D(mesh_.halfedge_handle(19))[0] ) << "Wrong texCoord at halfedge 4 component 0";
  EXPECT_EQ(6, mesh_.texcoord3D(mesh_.halfedge_handle(19))[1] ) << "Wrong texCoord at halfedge 4 component 1";
  EXPECT_EQ(6, mesh_.texcoord3D(mesh_.halfedge_handle(19))[2] ) << "Wrong texCoord at halfedge 4 component 2";

  EXPECT_EQ(7, mesh_.texcoord3D(mesh_.halfedge_handle(24))[0] ) << "Wrong texCoord at halfedge 7 component 0";
  EXPECT_EQ(7, mesh_.texcoord3D(mesh_.halfedge_handle(24))[1] ) << "Wrong texCoord at halfedge 7 component 1";
  EXPECT_EQ(7, mesh_.texcoord3D(mesh_.halfedge_handle(24))[2] ) << "Wrong texCoord at halfedge 7 component 2";

  EXPECT_EQ(9, mesh_.texcoord3D(mesh_.halfedge_handle(30))[0] ) << "Wrong texCoord at halfedge 9 component 0";
  EXPECT_EQ(9, mesh_.texcoord3D(mesh_.halfedge_handle(30))[1] ) << "Wrong texCoord at halfedge 9 component 1";
  EXPECT_EQ(9, mesh_.texcoord3D(mesh_.halfedge_handle(30))[2] ) << "Wrong texCoord at halfedge 9 component 2";

  EXPECT_EQ(12, mesh_.texcoord3D(mesh_.halfedge_handle(35))[0] ) << "Wrong texCoord at halfedge 11 component 0";
  EXPECT_EQ(12, mesh_.texcoord3D(mesh_.halfedge_handle(35))[1] ) << "Wrong texCoord at halfedge 11 component 1";
  EXPECT_EQ(12, mesh_.texcoord3D(mesh_.halfedge_handle(35))[2] ) << "Wrong texCoord at halfedge 11 component 2";

  mesh_.request_halfedge_texcoords3D();
}

/*
 * Just load a obj file of a square with a material
 */
TEST_F(OpenMeshReadWriteOBJ, LoadObjWithMaterial) {

  mesh_.clear();

  mesh_.request_face_colors();

  OpenMesh::IO::Options options;
  options += OpenMesh::IO::Options::FaceColor;

  std::string file_name = "square_material.obj";

  bool ok = OpenMesh::IO::read_mesh(mesh_, file_name,options);

  EXPECT_TRUE(ok) << file_name;

  OpenMesh::FaceHandle fh = mesh_.face_handle(mesh_.halfedge_handle(0));

  EXPECT_TRUE(fh.is_valid()) << "fh should be valid";

#ifdef TEST_DOUBLE_TRAITS
  EXPECT_FLOAT_EQ(128/255.0,   mesh_.color(fh)[0] ) << "Wrong vertex color at vertex 0 component 0";
  EXPECT_FLOAT_EQ(128/255.0,   mesh_.color(fh)[1] ) << "Wrong vertex color at vertex 0 component 1";
  EXPECT_FLOAT_EQ(128/255.0,   mesh_.color(fh)[2] ) << "Wrong vertex color at vertex 0 component 2";
  EXPECT_FLOAT_EQ(1.0,         mesh_.color(fh)[3] ) << "Wrong vertex color at vertex 0 component 3";
#else
  EXPECT_EQ(128,   mesh_.color(fh)[0] ) << "Wrong vertex color at vertex 0 component 0";
  EXPECT_EQ(128,   mesh_.color(fh)[1] ) << "Wrong vertex color at vertex 0 component 1";
  EXPECT_EQ(128,   mesh_.color(fh)[2] ) << "Wrong vertex color at vertex 0 component 2";
#endif

  mesh_.release_face_colors();
}

TEST_F(OpenMeshReadWriteOBJ, LoadObjWithTexture) {

    mesh_.clear();

    mesh_.request_face_colors();
    mesh_.request_face_texture_index();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::FaceTexCoord;

    std::string file_name = "square_material_texture.obj";

    bool ok = OpenMesh::IO::read_mesh(mesh_, file_name, options);

    EXPECT_TRUE(ok) << file_name;

    //check texture mapping for the mesh
    OpenMesh::MPropHandleT< std::map< int, std::string > > property;
    mesh_.get_property_handle(property, "TextureMapping");
    EXPECT_EQ(mesh_.property(property).size(), 1u) << "More than one texture defined";
    std::map< int, std::string >::iterator tex = mesh_.property(property).find(1);
    EXPECT_TRUE(tex != mesh_.property(property).end()) << "Could not find texture with id 1";
    EXPECT_TRUE((mesh_.property(property)[1] == std::string("square_material_texture.jpg"))) << "Wrong texture name";

    //check texture mapping per face
    OpenMesh::FaceHandle fh = mesh_.face_handle(mesh_.halfedge_handle(0));
    EXPECT_TRUE(fh.is_valid()) << "fh should be valid";
    EXPECT_EQ(mesh_.property(mesh_.face_texture_index_pph(),fh),1) << "Face texture index is not set correctly";

    mesh_.release_face_colors();
    mesh_.release_face_texture_index();
}

/*
 * Just load a obj file of a cube with vertex colors defined directly after the vertex definitions
 */
TEST_F(OpenMeshReadWriteOBJ, LoadSimpleOBJWithVertexColorsAfterVertices) {

    mesh_.clear();

    mesh_.request_vertex_colors();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::VertexColor;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-vertex-colors-after-vertex-definition.obj",options);

    EXPECT_TRUE(ok) << "Unable to load cube-minimal-vertex-colors-after-vertex-definition.obj";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces())    << "The number of loaded faces is not correct!";


#ifdef TEST_DOUBLE_TRAITS
    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#else
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#endif

    mesh_.release_vertex_colors();
}

/*
 * Just load a obj file of a cube with vertex colors defined as separate lines
 */
TEST_F(OpenMeshReadWriteOBJ, LoadSimpleOBJWithVertexColorsAsVCLines) {

    mesh_.clear();

    mesh_.request_vertex_colors();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::VertexColor;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-vertex-colors-as-vc-lines.obj",options);

    EXPECT_TRUE(ok) << "Unable to load cube-minimal-vertex-colors-as-vc-lines.obj";

    EXPECT_EQ(8u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(18u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(12u , mesh_.n_faces()) << "The number of loaded faces is not correct!";

#ifdef TEST_DOUBLE_TRAITS
    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_FLOAT_EQ(0.0,   mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_FLOAT_EQ(1.0, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#else
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[0] ) << "Wrong vertex color at vertex 0 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[1] ) << "Wrong vertex color at vertex 0 component 1";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(0))[2] ) << "Wrong vertex color at vertex 0 component 2";

    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(3))[0] ) << "Wrong vertex color at vertex 3 component 0";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[1] ) << "Wrong vertex color at vertex 3 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(3))[2] ) << "Wrong vertex color at vertex 3 component 2";

    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(4))[0] ) << "Wrong vertex color at vertex 4 component 0";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[1] ) << "Wrong vertex color at vertex 4 component 1";
    EXPECT_EQ(0,   mesh_.color(mesh_.vertex_handle(4))[2] ) << "Wrong vertex color at vertex 4 component 2";

    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[0] ) << "Wrong vertex color at vertex 7 component 0";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[1] ) << "Wrong vertex color at vertex 7 component 1";
    EXPECT_EQ(255, mesh_.color(mesh_.vertex_handle(7))[2] ) << "Wrong vertex color at vertex 7 component 2";
#endif

    mesh_.release_vertex_colors();

}

/*
 * Load, save and load a simple obj
 */
TEST_F(OpenMeshReadWriteOBJ, ReadWriteReadSimpleOBJ) {

  mesh_.clear();
  mesh_.request_vertex_normals();

  OpenMesh::IO::Options options;
  options += OpenMesh::IO::Options::VertexNormal;
  bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal.obj", options);

  EXPECT_TRUE(ok) << "Unable to load cube-minimal.obj";

  options.clear();
  options += OpenMesh::IO::Options::VertexNormal;
  const char* filename = "cube-minimal_openmeshOutputTestfile.obj";
  ok = OpenMesh::IO::write_mesh(mesh_, filename, options);

  mesh_.release_vertex_normals();
  ASSERT_TRUE(ok) << "Unable to write obj mesh";

  Mesh mesh2;
  mesh2.request_vertex_normals();

  options.clear();

  options += OpenMesh::IO::Options::VertexNormal;
  ok = OpenMesh::IO::read_mesh(mesh2, filename, options);
  remove(filename);
  ASSERT_TRUE(ok) << "Unable to read written mesh";

  EXPECT_EQ(8u  , mesh2.n_vertices()) << "The number of loaded vertices is not correct!";
  EXPECT_EQ(18u , mesh2.n_edges()) << "The number of loaded edges is not correct!";
  EXPECT_EQ(12u , mesh2.n_faces()) << "The number of loaded faces is not correct!";

  ///////////////////////////////////////////////
  //check vertex normals
  EXPECT_EQ(0,   mesh2.normal(mesh2.vertex_handle(0))[0] ) << "Wrong vertex normal at vertex 0 component 0";
  EXPECT_EQ(-1,   mesh2.normal(mesh2.vertex_handle(0))[1] ) << "Wrong vertex normal at vertex 0 component 1";
  EXPECT_EQ(0,   mesh2.normal(mesh2.vertex_handle(0))[2] ) << "Wrong vertex normal at vertex 0 component 2";

  EXPECT_EQ(0,   mesh2.normal(mesh2.vertex_handle(3))[0] ) << "Wrong vertex normal at vertex 3 component 0";
  EXPECT_EQ(0, mesh2.normal(mesh2.vertex_handle(3))[1] ) << "Wrong vertex normal at vertex 3 component 1";
  EXPECT_EQ(1, mesh2.normal(mesh2.vertex_handle(3))[2] ) << "Wrong vertex normal at vertex 3 component 2";

  EXPECT_EQ(0, mesh2.normal(mesh2.vertex_handle(4))[0] ) << "Wrong vertex normal at vertex 4 component 0";
  EXPECT_EQ(-1,   mesh2.normal(mesh2.vertex_handle(4))[1] ) << "Wrong vertex normal at vertex 4 component 1";
  EXPECT_EQ(0,   mesh2.normal(mesh2.vertex_handle(4))[2] ) << "Wrong vertex normal at vertex 4 component 2";

  EXPECT_EQ(0, mesh2.normal(mesh2.vertex_handle(7))[0] ) << "Wrong vertex normal at vertex 7 component 0";
  EXPECT_EQ(0, mesh2.normal(mesh2.vertex_handle(7))[1] ) << "Wrong vertex normal at vertex 7 component 1";
  EXPECT_EQ(1, mesh2.normal(mesh2.vertex_handle(7))[2] ) << "Wrong vertex normal at vertex 7 component 2";

}


TEST_F(OpenMeshReadWriteOBJ, FaceTexCoordTest) {

  mesh_.clear();
  mesh_.request_vertex_normals();
  mesh_.request_halfedge_texcoords2D();

  // Add some vertices
  Mesh::VertexHandle vhandle[5];

  vhandle[0] = mesh_.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh_.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh_.add_vertex(Mesh::Point(1, 1, 0));

  // Add one face
  std::vector<Mesh::VertexHandle> face_vhandles;

  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);

  Mesh::FaceHandle fh = mesh_.add_face(face_vhandles);

  //  1 --- 2
  //  |   /
  //  |  /
  //  | /
  //  0

  mesh_.set_normal(vhandle[0] , Mesh::Normal(1,0,0));
  mesh_.set_normal(vhandle[1] , Mesh::Normal(0,1,0));
  mesh_.set_normal(vhandle[2] , Mesh::Normal(0,0,1));


  float u = 8.0f;
  for ( auto he : mesh_.halfedges() ) {

    mesh_.set_texcoord2D(he,Mesh::TexCoord2D(u,u));
    u += 1.0;
  }


  u = 0.0f;

  for ( auto he : mesh_.fh_range(fh) )
  {

    mesh_.set_texcoord2D(he,Mesh::TexCoord2D(u,u));
    u += 1.0f;
  }



  OpenMesh::IO::Options wopt;
  wopt += OpenMesh::IO::Options::VertexNormal;
  wopt += OpenMesh::IO::Options::FaceTexCoord;

  bool ok = OpenMesh::IO::write_mesh(mesh_, "OpenMeshReadWriteOBJ_FaceTexCoordTest.obj", wopt);

  EXPECT_TRUE(ok) << "Unable to write OpenMeshReadWriteOBJ_FaceTexCoordTest.obj";

  mesh_.clear();


  OpenMesh::IO::Options ropt;

  ropt += OpenMesh::IO::Options::FaceTexCoord;

  mesh_.request_vertex_normals();
  mesh_.request_face_normals();
  mesh_.request_halfedge_texcoords2D();

  ok = OpenMesh::IO::read_mesh(mesh_, "OpenMeshReadWriteOBJ_FaceTexCoordTest.obj", ropt);

  EXPECT_TRUE(ok) << "Unable to read back OpenMeshReadWriteOBJ_FaceTexCoordTest.obj";

}






}

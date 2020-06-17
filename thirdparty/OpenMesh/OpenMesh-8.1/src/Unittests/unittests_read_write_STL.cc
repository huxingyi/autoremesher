
#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>


namespace {

class OpenMeshReadWriteSTL : public OpenMeshBase {

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
 * Just load a simple mesh file in stla format and count whether
 * the right number of entities has been loaded.
 */
TEST_F(OpenMeshReadWriteSTL, LoadSimpleSTLFile) {

    mesh_.clear();

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.stl");

    EXPECT_TRUE(ok);

    EXPECT_EQ(7526u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(22572u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(15048u , mesh_.n_faces()) << "The number of loaded faces is not correct!";
}


/*
 * Just load a simple mesh file in stla format and count whether
 * the right number of entities has been loaded. Also check facet normals.
 */
TEST_F(OpenMeshReadWriteSTL, LoadSimpleSTLFileWithNormals) {

    mesh_.clear();
    mesh_.request_face_normals();

    OpenMesh::IO::Options opt;
    opt += OpenMesh::IO::Options::FaceNormal;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1.stl", opt);

    EXPECT_TRUE(ok);

    EXPECT_TRUE(opt.face_has_normal());
    EXPECT_FALSE(opt.vertex_has_normal());

    EXPECT_NEAR(-0.038545f, mesh_.normal(mesh_.face_handle(0))[0], 0.0001 ) << "Wrong face normal at face 0 component 0";
    EXPECT_NEAR(-0.004330f, mesh_.normal(mesh_.face_handle(0))[1], 0.0001 ) << "Wrong face normal at face 0 component 1";
    EXPECT_NEAR(0.999247f, mesh_.normal(mesh_.face_handle(0))[2], 0.0001 ) << "Wrong face normal at face 0 component 2";

    EXPECT_EQ(7526u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(22572u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(15048u , mesh_.n_faces()) << "The number of loaded faces is not correct!";

    mesh_.release_face_normals();
}


/*
 * Just load a simple mesh file in stlb format and count whether
 * the right number of entities has been loaded.
 */
TEST_F(OpenMeshReadWriteSTL, LoadSimpleSTLBinaryFile) {

    mesh_.clear();

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1Binary.stl");

    EXPECT_TRUE(ok);

    EXPECT_EQ(7526u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(22572u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(15048u , mesh_.n_faces()) << "The number of loaded faces is not correct!";
}


/*
 * Just load a simple mesh file in stlb format and count whether
 * the right number of entities has been loaded. Also check facet normals.
 */
TEST_F(OpenMeshReadWriteSTL, LoadSimpleSTLBinaryFileWithNormals) {

    mesh_.clear();
    mesh_.request_face_normals();

    OpenMesh::IO::Options opt;
    opt += OpenMesh::IO::Options::FaceNormal;
    opt += OpenMesh::IO::Options::Binary;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1Binary.stl", opt);

    EXPECT_TRUE(ok);

    EXPECT_TRUE(opt.is_binary());
    EXPECT_TRUE(opt.face_has_normal());
    EXPECT_FALSE(opt.vertex_has_normal());

    EXPECT_NEAR(-0.038545f, mesh_.normal(mesh_.face_handle(0))[0], 0.0001 ) << "Wrong face normal at face 0 component 0";
    EXPECT_NEAR(-0.004330f, mesh_.normal(mesh_.face_handle(0))[1], 0.0001 ) << "Wrong face normal at face 0 component 1";
    EXPECT_NEAR(0.999247f, mesh_.normal(mesh_.face_handle(0))[2], 0.0001 ) << "Wrong face normal at face 0 component 2";

    EXPECT_EQ(7526u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(22572u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(15048u , mesh_.n_faces()) << "The number of loaded faces is not correct!";

    mesh_.release_face_normals();
}

/*
 * Read and Write stl binary file
 */
TEST_F(OpenMeshReadWriteSTL, ReadWriteSimpleSTLBinaryFile) {

    mesh_.clear();

    OpenMesh::IO::Options opt;
    opt += OpenMesh::IO::Options::Binary;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1Binary.stl");

    EXPECT_TRUE(ok);

    const char* filename = "cube1Binary_openmeshWriteTestFile.stl";

    ok = OpenMesh::IO::write_mesh(mesh_, filename, opt);

    EXPECT_TRUE(ok);

    ok = OpenMesh::IO::read_mesh(mesh_, filename, opt);

    EXPECT_TRUE(ok);

    EXPECT_EQ(7526u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(22572u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(15048u , mesh_.n_faces()) << "The number of loaded faces is not correct!";

    remove(filename);
}

/*
 * Just load a simple mesh file in stlb format rewrite and load it again and count whether
 * the right number of entities has been loaded. Also check facet normals.
 */
TEST_F(OpenMeshReadWriteSTL, ReadWriteSimpleSTLBinaryFileWithNormals) {

    mesh_.clear();
    mesh_.request_face_normals();

    OpenMesh::IO::Options opt;
    opt += OpenMesh::IO::Options::FaceNormal;
    opt += OpenMesh::IO::Options::Binary;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1Binary.stl", opt);

    EXPECT_TRUE(ok);

    const char* filename = "cube1BinaryNormal_openmeshWriteTestFile.stl";

    ok = OpenMesh::IO::write_mesh(mesh_, filename, opt);

    EXPECT_TRUE(ok);

    ok = OpenMesh::IO::read_mesh(mesh_, filename, opt);

    EXPECT_TRUE(ok);

    EXPECT_TRUE(opt.is_binary());
    EXPECT_TRUE(opt.face_has_normal());
    EXPECT_FALSE(opt.vertex_has_normal());

    EXPECT_NEAR(-0.038545f, mesh_.normal(mesh_.face_handle(0))[0], 0.0001 ) << "Wrong face normal at face 0 component 0";
    EXPECT_NEAR(-0.004330f, mesh_.normal(mesh_.face_handle(0))[1], 0.0001 ) << "Wrong face normal at face 0 component 1";
    EXPECT_NEAR(0.999247f, mesh_.normal(mesh_.face_handle(0))[2], 0.0001 ) << "Wrong face normal at face 0 component 2";

    EXPECT_EQ(7526u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(22572u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(15048u , mesh_.n_faces()) << "The number of loaded faces is not correct!";

    mesh_.release_face_normals();
    remove(filename);
}

/*
 * Just load a simple mesh file in stlb format rewrite and load it again and count whether
 * the right number of entities has been loaded. Also check facet normals.
 */
TEST_F(OpenMeshReadWriteSTL, ReadWriteSimpleSTLAsciiFileWithNormals) {

    mesh_.clear();
    mesh_.request_face_normals();

    OpenMesh::IO::Options opt;
    opt += OpenMesh::IO::Options::FaceNormal;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube1Binary.stl", opt);

    EXPECT_TRUE(ok);
    opt.clear();
    opt += OpenMesh::IO::Options::FaceNormal;
    const char* filename = "cube1Normal_openmeshWriteTestFile.stl";

    ok = OpenMesh::IO::write_mesh(mesh_, filename, opt);

    EXPECT_TRUE(ok);

    opt.clear();
    opt += OpenMesh::IO::Options::FaceNormal;
    ok = OpenMesh::IO::read_mesh(mesh_, filename, opt);

    EXPECT_TRUE(ok);

    EXPECT_FALSE(opt.is_binary());
    EXPECT_TRUE(opt.face_has_normal());
    EXPECT_FALSE(opt.vertex_has_normal());

    EXPECT_NEAR(-0.038545f, mesh_.normal(mesh_.face_handle(0))[0], 0.0001 ) << "Wrong face normal at face 0 component 0";
    EXPECT_NEAR(-0.004330f, mesh_.normal(mesh_.face_handle(0))[1], 0.0001 ) << "Wrong face normal at face 0 component 1";
    EXPECT_NEAR(0.999247f, mesh_.normal(mesh_.face_handle(0))[2], 0.0001 ) << "Wrong face normal at face 0 component 2";

    EXPECT_EQ(7526u  , mesh_.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(22572u , mesh_.n_edges()) << "The number of loaded edges is not correct!";
    EXPECT_EQ(15048u , mesh_.n_faces()) << "The number of loaded faces is not correct!";

    mesh_.release_face_normals();
    remove(filename);
}


}

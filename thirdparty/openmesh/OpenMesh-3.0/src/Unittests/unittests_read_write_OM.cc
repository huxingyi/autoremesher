#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>


namespace {

class OpenMeshReadWriteOM : public OpenMeshBase {

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
 * Just load an om file and set vertex color option before loading
 */
TEST_F(OpenMeshReadWriteOM, LoadSimpleOMForceVertexColorsAlthoughNotAvailable) {

  mesh_.clear();

  mesh_.request_vertex_colors();

  std::string file_name = "cube-minimal.om";

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
 * Just load an om file of a cube with vertex texCoords
 */
TEST_F(OpenMeshReadWriteOM, LoadSimpleOMWithTexCoords) {

    mesh_.clear();

    mesh_.request_vertex_texcoords2D();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::VertexTexCoord;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-texCoords.om",options);

    EXPECT_TRUE(ok) << "Unable to load cube-minimal-texCoords.om";

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
 * Just load an om file of a cube with vertex colors
 */
TEST_F(OpenMeshReadWriteOM, LoadSimpleOMWithVertexColors) {

    mesh_.clear();

    mesh_.request_vertex_colors();

    OpenMesh::IO::Options options;
    options += OpenMesh::IO::Options::VertexColor;

    bool ok = OpenMesh::IO::read_mesh(mesh_, "cube-minimal-vertexColors.om",options);

    EXPECT_TRUE(ok) << "Unable to load cube-minimal-vertexColors.om";

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
 * Save and load simple mesh
 */
TEST_F(OpenMeshReadWriteOM, WriteTriangle) {

  Mesh mesh;
  mesh.clear();

  const std::string filename = "triangle-minimal.om";

  // generate data
  Mesh::VertexHandle v1 = mesh.add_vertex(Mesh::Point(1.0,0.0,0.0));
  Mesh::VertexHandle v2 = mesh.add_vertex(Mesh::Point(0.0,1.0,0.0));
  Mesh::VertexHandle v3 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
  mesh.add_face(v1,v2,v3);

  // save
  bool ok = OpenMesh::IO::write_mesh(mesh,filename);
  EXPECT_TRUE(ok) << "Unable to write " << filename;

  // reset
  mesh.clear();

  // load
  ok = OpenMesh::IO::read_mesh(mesh,filename);
  EXPECT_TRUE(ok) << "Unable to read " << filename;

  // compare
  EXPECT_EQ(3u  , mesh.n_vertices()) << "The number of loaded vertices is not correct!";
  EXPECT_EQ(3u , mesh.n_edges())    << "The number of loaded edges is not correct!";
  EXPECT_EQ(1u , mesh.n_faces())    << "The number of loaded faces is not correct!";

  EXPECT_EQ(Mesh::Point(1.0,0.0,0.0) , mesh.point(v1)) << "Wrong coordinates at vertex 0";
  EXPECT_EQ(Mesh::Point(0.0,1.0,0.0) , mesh.point(v2)) << "Wrong coordinates at vertex 1";
  EXPECT_EQ(Mesh::Point(0.0,0.0,1.0) , mesh.point(v3)) << "Wrong coordinates at vertex 2";

  // cleanup
  remove(filename.c_str());

}

/*
 * Save and load simple mesh with integer colors per vertex
 */
TEST_F(OpenMeshReadWriteOM, WriteTriangleVertexIntegerColor) {

  Mesh mesh;

  mesh.request_vertex_colors();

  OpenMesh::IO::Options options;
  options += OpenMesh::IO::Options::VertexColor;
  options -= OpenMesh::IO::Options::ColorFloat;

  const std::string filename = "triangle-minimal-ColorsPerVertex.om";

  // generate data
  Mesh::VertexHandle v1 = mesh.add_vertex(Mesh::Point(1.0,0.0,0.0));
  Mesh::VertexHandle v2 = mesh.add_vertex(Mesh::Point(0.0,1.0,0.0));
  Mesh::VertexHandle v3 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
  mesh.add_face(v1,v2,v3);

  Mesh::Color c1 = Mesh::Color(0,0,123),
      c2 = Mesh::Color(21,0,0),
      c3 = Mesh::Color(0,222,0);

  mesh.set_color(v1,c1);
  mesh.set_color(v2,c2);
  mesh.set_color(v3,c3);

  // save
  bool ok = OpenMesh::IO::write_mesh(mesh,filename,options);
  EXPECT_TRUE(ok) << "Unable to write "<<filename;

  mesh.release_vertex_colors();

  // load
  Mesh cmpMesh;
  cmpMesh.request_vertex_colors();
  ok = OpenMesh::IO::read_mesh(cmpMesh,filename,options);
  EXPECT_TRUE(ok) << "Unable to read "<<filename;

  EXPECT_TRUE(cmpMesh.has_vertex_colors()) << "Loaded mesh has no vertex colors.";

  // compare
  EXPECT_EQ(3u  , cmpMesh.n_vertices()) << "The number of loaded vertices is not correct!";
  EXPECT_EQ(3u , cmpMesh.n_edges())    << "The number of loaded edges is not correct!";
  EXPECT_EQ(1u , cmpMesh.n_faces())    << "The number of loaded faces is not correct!";

  EXPECT_EQ(Mesh::Point(1.0,0.0,0.0) , cmpMesh.point(v1)) << "Wrong coordinates at vertex 0";
  EXPECT_EQ(Mesh::Point(0.0,1.0,0.0) , cmpMesh.point(v2)) << "Wrong coordinates at vertex 1";
  EXPECT_EQ(Mesh::Point(0.0,0.0,1.0) , cmpMesh.point(v3)) << "Wrong coordinates at vertex 2";

  EXPECT_EQ(c1 , cmpMesh.color(v1)) << "Wrong colors at vertex 0";
  EXPECT_EQ(c2 , cmpMesh.color(v2)) << "Wrong colors at vertex 1";
  EXPECT_EQ(c3 , cmpMesh.color(v3)) << "Wrong colors at vertex 2";

  //clean up
  cmpMesh.release_vertex_colors();
  remove(filename.c_str());

}

/*
 * Save and load simple mesh with custom property
 */
TEST_F(OpenMeshReadWriteOM, WriteTriangleVertexBoolProperty) {

    Mesh mesh;

    const std::string filename = "triangle-minimal-VBProp.om";

    // generate data
    Mesh::VertexHandle v1 = mesh.add_vertex(Mesh::Point(1.0,0.0,0.0));
    Mesh::VertexHandle v2 = mesh.add_vertex(Mesh::Point(0.0,1.0,0.0));
    Mesh::VertexHandle v3 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    mesh.add_face(v1,v2,v3);

    OpenMesh::VPropHandleT<bool> prop;
    mesh.add_property(prop,"VBProp");
    mesh.property(prop).set_persistent(true);

    mesh.property(prop,v1) = true;
    mesh.property(prop,v2) = false;
    mesh.property(prop,v3) = true;

    // save
    bool ok = OpenMesh::IO::write_mesh(mesh,filename);
    EXPECT_TRUE(ok) << "Unable to write "<<filename;

    // load
    Mesh cmpMesh;


    cmpMesh.add_property(prop,"VBProp");
    cmpMesh.property(prop).set_persistent(true);

    ok = OpenMesh::IO::read_mesh(cmpMesh,filename);
    EXPECT_TRUE(ok) << "Unable to read "<<filename;

    // compare
    EXPECT_EQ(3u  , cmpMesh.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(3u , cmpMesh.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(1u , cmpMesh.n_faces())    << "The number of loaded faces is not correct!";

    EXPECT_EQ(Mesh::Point(1.0,0.0,0.0) , cmpMesh.point(v1)) << "Wrong coordinates at vertex 0";
    EXPECT_EQ(Mesh::Point(0.0,1.0,0.0) , cmpMesh.point(v2)) << "Wrong coordinates at vertex 1";
    EXPECT_EQ(Mesh::Point(0.0,0.0,1.0) , cmpMesh.point(v3)) << "Wrong coordinates at vertex 2";

    EXPECT_TRUE(cmpMesh.property(prop,v1)) << "Wrong Property value at vertex 0";
    EXPECT_FALSE(cmpMesh.property(prop,v2)) << "Wrong Property value at vertex 1";
    EXPECT_TRUE(cmpMesh.property(prop,v3)) << "Wrong Property value at vertex 2";

    // cleanup
    remove(filename.c_str());

}

/*
 * Save and load simple mesh with custom property
 */
TEST_F(OpenMeshReadWriteOM, WriteTriangleVertexBoolPropertySpaceEquivalent) {

    Mesh mesh;

    const std::string filename = "triangle-minimal-VBProp-pattern-test.om";

    // generate data
    Mesh::VertexHandle v1 = mesh.add_vertex(Mesh::Point(1.0,0.0,0.0));
    Mesh::VertexHandle v2 = mesh.add_vertex(Mesh::Point(0.0,1.0,0.0));
    Mesh::VertexHandle v3 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    mesh.add_face(v1,v2,v3);

    Mesh::VertexHandle v4 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    Mesh::VertexHandle v5 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    Mesh::VertexHandle v6 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    mesh.add_face(v4,v5,v6);

    Mesh::VertexHandle v7 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    Mesh::VertexHandle v8 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    Mesh::VertexHandle v9 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));

    OpenMesh::VPropHandleT<bool> prop;
    mesh.add_property(prop,"VBProp");
    mesh.property(prop).set_persistent(true);

    // Create a 0x20 hex pattern in the bitset
    mesh.property(prop,v1) = false;
    mesh.property(prop,v2) = false;
    mesh.property(prop,v3) = false;
    mesh.property(prop,v4) = false;
    mesh.property(prop,v5) = false;
    mesh.property(prop,v6) = true;
    mesh.property(prop,v7) = false;
    mesh.property(prop,v8) = false;
    mesh.property(prop,v9) = true;

    // save
    bool ok = OpenMesh::IO::write_mesh(mesh,filename);
    EXPECT_TRUE(ok) << "Unable to write "<<filename;

    // load
    Mesh cmpMesh;

    cmpMesh.add_property(prop,"VBProp");
    cmpMesh.property(prop).set_persistent(true);

    ok = OpenMesh::IO::read_mesh(cmpMesh,filename);
    EXPECT_TRUE(ok) << "Unable to read "<<filename;

    // compare
    EXPECT_EQ(9u , cmpMesh.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(6u , cmpMesh.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(2u , cmpMesh.n_faces())    << "The number of loaded faces is not correct!";

    EXPECT_FALSE(cmpMesh.property(prop,v1)) << "Wrong Property value at vertex 0";
    EXPECT_FALSE(cmpMesh.property(prop,v2)) << "Wrong Property value at vertex 1";
    EXPECT_FALSE(cmpMesh.property(prop,v3)) << "Wrong Property value at vertex 2";
    EXPECT_FALSE(cmpMesh.property(prop,v4)) << "Wrong Property value at vertex 3";
    EXPECT_FALSE(cmpMesh.property(prop,v5)) << "Wrong Property value at vertex 4";
    EXPECT_TRUE(cmpMesh.property(prop,v6))  << "Wrong Property value at vertex 5";
    EXPECT_FALSE(cmpMesh.property(prop,v7)) << "Wrong Property value at vertex 6";
    EXPECT_FALSE(cmpMesh.property(prop,v8)) << "Wrong Property value at vertex 7";
    EXPECT_TRUE(cmpMesh.property(prop,v9))  << "Wrong Property value at vertex 8";

    // cleanup
    remove(filename.c_str());

}

/*
 * Save and load simple mesh with multiple custom property
 */
TEST_F(OpenMeshReadWriteOM, WriteTriangleTwoVertexBoolProperty) {

    Mesh mesh;

    const std::string filename = "triangle-minimal-VBProp.om";

    // generate data
    Mesh::VertexHandle v1 = mesh.add_vertex(Mesh::Point(1.0,0.0,0.0));
    Mesh::VertexHandle v2 = mesh.add_vertex(Mesh::Point(0.0,1.0,0.0));
    Mesh::VertexHandle v3 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    mesh.add_face(v1,v2,v3);

    OpenMesh::VPropHandleT<bool> prop;
    mesh.add_property(prop,"VBProp");
    mesh.property(prop).set_persistent(true);

    mesh.property(prop,v1) = true;
    mesh.property(prop,v2) = false;
    mesh.property(prop,v3) = true;

    OpenMesh::VPropHandleT<bool> prop2;
    mesh.add_property(prop2,"VBProp2");
    mesh.property(prop2).set_persistent(true);

    mesh.property(prop2,v1) = false;
    mesh.property(prop2,v2) = false;
    mesh.property(prop2,v3) = false;

    // save
    bool ok = OpenMesh::IO::write_mesh(mesh,filename);
    EXPECT_TRUE(ok) << "Unable to write "<<filename;


    // load
    Mesh cmpMesh;
    cmpMesh.add_property(prop,"VBProp");
    cmpMesh.property(prop).set_persistent(true);

    cmpMesh.add_property(prop2,"VBProp2");
    cmpMesh.property(prop2).set_persistent(true);

    ok = OpenMesh::IO::read_mesh(cmpMesh,filename);
    EXPECT_TRUE(ok) << "Unable to read "<<filename;

    // compare
    EXPECT_EQ(3u  , cmpMesh.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(3u , cmpMesh.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(1u , cmpMesh.n_faces())    << "The number of loaded faces is not correct!";

    EXPECT_EQ(Mesh::Point(1.0,0.0,0.0) , cmpMesh.point(v1)) << "Wrong coordinates at vertex 0";
    EXPECT_EQ(Mesh::Point(0.0,1.0,0.0) , cmpMesh.point(v2)) << "Wrong coordinates at vertex 1";
    EXPECT_EQ(Mesh::Point(0.0,0.0,1.0) , cmpMesh.point(v3)) << "Wrong coordinates at vertex 2";

    EXPECT_TRUE(cmpMesh.property(prop,v1)) << "Wrong Property value at vertex 0";
    EXPECT_FALSE(cmpMesh.property(prop,v2)) << "Wrong Property value at vertex 1";
    EXPECT_TRUE(cmpMesh.property(prop,v3)) << "Wrong Property value at vertex 2";

    EXPECT_FALSE(cmpMesh.property(prop2,v1)) << "Wrong second Property value at vertex 0";
    EXPECT_FALSE(cmpMesh.property(prop2,v2)) << "Wrong second Property value at vertex 1";
    EXPECT_FALSE(cmpMesh.property(prop2,v3)) << "Wrong second Property value at vertex 2";

    // cleanup
    remove(filename.c_str());

}

/*
 * Save and load simple mesh with custom property
 */
TEST_F(OpenMeshReadWriteOM, WriteTriangleEdgeIntProperty) {

    Mesh mesh;

    const std::string propName = "EIProp";
    const std::string filename = std::string("triangle-minimal-")+propName+".om";

    // generate data
    Mesh::VertexHandle v1 = mesh.add_vertex(Mesh::Point(1.0,0.0,0.0));
    Mesh::VertexHandle v2 = mesh.add_vertex(Mesh::Point(0.0,1.0,0.0));
    Mesh::VertexHandle v3 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    mesh.add_face(v1,v2,v3);

    OpenMesh::EPropHandleT<int> prop;
    mesh.add_property(prop,propName);
    mesh.property(prop).set_persistent(true);

    Mesh::EdgeHandle e1 = Mesh::EdgeHandle(0);
    Mesh::EdgeHandle e2 = Mesh::EdgeHandle(1);
    Mesh::EdgeHandle e3 = Mesh::EdgeHandle(2);

    int va1ue1 = 10,
        value2 = 21,
        value3 = 32;

    mesh.property(prop,e1) = va1ue1;
    mesh.property(prop,e2) = value2;
    mesh.property(prop,e3) = value3;

    // save
    bool ok = OpenMesh::IO::write_mesh(mesh,filename);
    EXPECT_TRUE(ok) << "Unable to write "<<filename;

    // load
    Mesh cmpMesh;

    cmpMesh.add_property(prop,propName);
    cmpMesh.property(prop).set_persistent(true);

    ok = OpenMesh::IO::read_mesh(cmpMesh,filename);
    EXPECT_TRUE(ok) << "Unable to read "<<filename;

    // compare
    EXPECT_EQ(3u  , cmpMesh.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(3u , cmpMesh.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(1u , cmpMesh.n_faces())    << "The number of loaded faces is not correct!";

    EXPECT_EQ(Mesh::Point(1.0,0.0,0.0) , cmpMesh.point(v1)) << "Wrong coordinates at vertex 0";
    EXPECT_EQ(Mesh::Point(0.0,1.0,0.0) , cmpMesh.point(v2)) << "Wrong coordinates at vertex 1";
    EXPECT_EQ(Mesh::Point(0.0,0.0,1.0) , cmpMesh.point(v3)) << "Wrong coordinates at vertex 2";

    EXPECT_EQ(va1ue1 , cmpMesh.property(prop,e1)) << "Wrong property at edge 0";
    EXPECT_EQ(value2 , cmpMesh.property(prop,e2)) << "Wrong property at edge 1";
    EXPECT_EQ(value3 , cmpMesh.property(prop,e3)) << "Wrong property at edge 2";

    // cleanup
    remove(filename.c_str());

}

/*
 * Save and load simple mesh with custom property
 */
TEST_F(OpenMeshReadWriteOM, WriteTriangleFaceDoubleProperty) {

    Mesh mesh;

    const std::string propName = "FDProp";
    const std::string filename = std::string("triangle-minimal-")+propName+".om";

    // generate data
    Mesh::VertexHandle v1 = mesh.add_vertex(Mesh::Point(1.0,0.0,0.0));
    Mesh::VertexHandle v2 = mesh.add_vertex(Mesh::Point(0.0,1.0,0.0));
    Mesh::VertexHandle v3 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    mesh.add_face(v1,v2,v3);

    OpenMesh::FPropHandleT<double> prop;
    mesh.add_property(prop,propName);
    mesh.property(prop).set_persistent(true);

    Mesh::FaceHandle f1 = Mesh::FaceHandle(0);

    double va1ue1 = 0.5;

    mesh.property(prop,f1) = va1ue1;

    // save
    bool ok = OpenMesh::IO::write_mesh(mesh,filename);
    EXPECT_TRUE(ok) << "Unable to write "<<filename;

    // load
    Mesh cmpMesh;

    cmpMesh.add_property(prop,propName);
    cmpMesh.property(prop).set_persistent(true);

    ok = OpenMesh::IO::read_mesh(cmpMesh,filename);
    EXPECT_TRUE(ok) << "Unable to read "<<filename;

    // compare
    EXPECT_EQ(3u  , cmpMesh.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(3u , cmpMesh.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(1u , cmpMesh.n_faces())    << "The number of loaded faces is not correct!";

    EXPECT_EQ(Mesh::Point(1.0,0.0,0.0) , cmpMesh.point(v1)) << "Wrong coordinates at vertex 0";
    EXPECT_EQ(Mesh::Point(0.0,1.0,0.0) , cmpMesh.point(v2)) << "Wrong coordinates at vertex 1";
    EXPECT_EQ(Mesh::Point(0.0,0.0,1.0) , cmpMesh.point(v3)) << "Wrong coordinates at vertex 2";

    EXPECT_EQ(va1ue1 , cmpMesh.property(prop,f1)) << "Wrong property at edge 0";

    // cleanup
    remove(filename.c_str());

}

/*
 * Save and load simple mesh with custom property
 */
TEST_F(OpenMeshReadWriteOM, WriteTriangleFaceFloatProperty) {

    const std::string propName = "FFProp";
    const std::string filename = std::string("triangle-minimal-")+propName+".om";

    // generate data
    Mesh mesh;
    Mesh::VertexHandle v1 = mesh.add_vertex(Mesh::Point(1.0,0.0,0.0));
    Mesh::VertexHandle v2 = mesh.add_vertex(Mesh::Point(0.0,1.0,0.0));
    Mesh::VertexHandle v3 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    mesh.add_face(v1,v2,v3);

    OpenMesh::FPropHandleT<float> prop;
    mesh.add_property(prop,propName);
    mesh.property(prop).set_persistent(true);

    Mesh::FaceHandle f1 = Mesh::FaceHandle(0);

    double va1ue1 = 3.1f;

    mesh.property(prop,f1) = va1ue1;

    // save
    bool ok = OpenMesh::IO::write_mesh(mesh,filename);
    EXPECT_TRUE(ok) << "Unable to write "<<filename;


    // load
    Mesh cmpMesh;
    cmpMesh.add_property(prop,propName);
    cmpMesh.property(prop).set_persistent(true);

    ok = OpenMesh::IO::read_mesh(cmpMesh,filename);
    EXPECT_TRUE(ok) << "Unable to read "<<filename;

    // compare
    EXPECT_EQ(3u  , cmpMesh.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(3u , cmpMesh.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(1u , cmpMesh.n_faces())    << "The number of loaded faces is not correct!";

    EXPECT_EQ(Mesh::Point(1.0,0.0,0.0) , cmpMesh.point(v1)) << "Wrong coordinates at vertex 0";
    EXPECT_EQ(Mesh::Point(0.0,1.0,0.0) , cmpMesh.point(v2)) << "Wrong coordinates at vertex 1";
    EXPECT_EQ(Mesh::Point(0.0,0.0,1.0) , cmpMesh.point(v3)) << "Wrong coordinates at vertex 2";

    EXPECT_EQ(va1ue1 , cmpMesh.property(prop,f1)) << "Wrong property at edge 0";

    // cleanup
    remove(filename.c_str());

}

/*
 * Save and load simple mesh with custom property
 */
TEST_F(OpenMeshReadWriteOM, ReadBigMeshWithCustomProperty) {

  OpenMesh::FPropHandleT<double> faceProp;
  OpenMesh::VPropHandleT<int> vertexProp;
  bool ok;

  //generate file
 /* mesh_.clear();
  ok = OpenMesh::IO::read_mesh(mesh_,"cube1.off");

  mesh_.add_property(faceProp,"DFProp");
  mesh_.property(faceProp).set_persistent(true);

  mesh_.add_property(vertexProp, "IVProp");
  mesh_.property(vertexProp).set_persistent(true);


  for (Mesh::FaceIter fIter = mesh_.faces_begin(); fIter != mesh_.faces_end(); ++fIter)
    mesh_.property(faceProp,*fIter) = 0.3;

  for (Mesh::VertexIter vIter = mesh_.vertices_begin(); vIter != mesh_.vertices_end(); ++vIter)
    mesh_.property(vertexProp,*vIter) = vIter->idx();

  OpenMesh::IO::write_mesh(mesh_,"cube1_customProps.om");


  mesh_.clear();
*/
  //read file
  Mesh mesh;
  mesh.add_property(faceProp,"DFProp");
  mesh.property(faceProp).set_persistent(true);

  mesh.add_property(vertexProp, "IVProp");
  mesh.property(vertexProp).set_persistent(true);

  ok = OpenMesh::IO::read_mesh(mesh,"cube1_customProps.om");
  EXPECT_TRUE(ok) << "Unable to read cube1_customProps.om";

  ///=============== result ======================
  EXPECT_EQ(7526u , mesh.n_vertices()) << "The number of loaded vertices is not correct!";
  EXPECT_EQ(22572u, mesh.n_edges()) << "The number of loaded edges is not correct!";
  EXPECT_EQ(15048u, mesh.n_faces()) << "The number of loaded faces is not correct!";

  bool wrong = false;
  for (Mesh::FaceIter fIter = mesh.faces_begin(); fIter != mesh.faces_end() && !wrong; ++fIter)
    wrong = (0.3 != mesh.property(faceProp,*fIter));
  EXPECT_FALSE(wrong) << "min one face has wrong face property";

  wrong = false;
  for (Mesh::VertexIter vIter = mesh.vertices_begin(); vIter != mesh.vertices_end() && !wrong; ++vIter)
    wrong = (vIter->idx() != mesh.property(vertexProp,*vIter));
  EXPECT_FALSE(wrong) << "min one vertex has worng vertex property";
}

}

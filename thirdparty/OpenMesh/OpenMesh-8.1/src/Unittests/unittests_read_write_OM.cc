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

    ASSERT_TRUE(ok) << "Unable to load cube-minimal-texCoords.om";

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

#ifdef TEST_DOUBLE_TRAITS
  Mesh::Color c1 = Mesh::Color(0,0,123/255.0,1.0),
      c2 = Mesh::Color(21/255.0,0,0,1.0),
      c3 = Mesh::Color(0,222/255.0,0,1.0);
#else
  Mesh::Color c1 = Mesh::Color(0,0,123),
      c2 = Mesh::Color(21,0,0),
      c3 = Mesh::Color(0,222,0);
#endif

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

#ifdef TEST_DOUBLE_TRAITS
  // OM file format does not support writing colors as float. They are stored as unsigned character.
  // Thus, the values will not be exactly equal.
  for (size_t i = 0; i < c1.size(); ++i)
  {
    EXPECT_FLOAT_EQ(c1[i] , cmpMesh.color(v1)[i]) << "Wrong colors at coordinate " << i << " of vertex 0";
    EXPECT_FLOAT_EQ(c2[i] , cmpMesh.color(v2)[i]) << "Wrong colors at coordinate " << i << " of vertex 1";
    EXPECT_FLOAT_EQ(c3[i] , cmpMesh.color(v3)[i]) << "Wrong colors at coordinate " << i << " of vertex 2";
  }
#else
  EXPECT_EQ(c1 , cmpMesh.color(v1)) << "Wrong colors at vertex 0";
  EXPECT_EQ(c2 , cmpMesh.color(v2)) << "Wrong colors at vertex 1";
  EXPECT_EQ(c3 , cmpMesh.color(v3)) << "Wrong colors at vertex 2";
#endif

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

    int value1 = 10,
        value2 = 21,
        value3 = 32;

    mesh.property(prop,e1) = value1;
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

    EXPECT_EQ(value1 , cmpMesh.property(prop,e1)) << "Wrong property at edge 0";
    EXPECT_EQ(value2 , cmpMesh.property(prop,e2)) << "Wrong property at edge 1";
    EXPECT_EQ(value3 , cmpMesh.property(prop,e3)) << "Wrong property at edge 2";

    // cleanup
    remove(filename.c_str());

}

/*
 * Save and load simple mesh with custom property
 */
TEST_F(OpenMeshReadWriteOM, WriteSplitTriangleEdgeIntProperty) {

    Mesh mesh;

    const std::string propName = "EIProp";
    const std::string filename = std::string("triangle-minimal-")+propName+".om";

    // generate data
    Mesh::VertexHandle v1 = mesh.add_vertex(Mesh::Point(1.0,0.0,0.0));
    Mesh::VertexHandle v2 = mesh.add_vertex(Mesh::Point(0.0,1.0,0.0));
    Mesh::VertexHandle v3 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    auto fh0 = mesh.add_face(v1,v2,v3);
    auto c = mesh.calc_face_centroid(fh0);
    Mesh::VertexHandle v4 = mesh.add_vertex(c);
    mesh.split(fh0, v4);


    OpenMesh::EPropHandleT<int> prop;
    mesh.add_property(prop,propName);
    mesh.property(prop).set_persistent(true);

    Mesh::EdgeHandle e1 = Mesh::EdgeHandle(0);
    Mesh::EdgeHandle e2 = Mesh::EdgeHandle(1);
    Mesh::EdgeHandle e3 = Mesh::EdgeHandle(2);
    Mesh::EdgeHandle e4 = Mesh::EdgeHandle(3);
    Mesh::EdgeHandle e5 = Mesh::EdgeHandle(4);
    Mesh::EdgeHandle e6 = Mesh::EdgeHandle(5);

    int value1 = 10,
        value2 = 21,
        value3 = 32,
        value4 = 43,
        value5 = 54,
        value6 = 65;

    mesh.property(prop,e1) = value1;
    mesh.property(prop,e2) = value2;
    mesh.property(prop,e3) = value3;
    mesh.property(prop,e4) = value4;
    mesh.property(prop,e5) = value5;
    mesh.property(prop,e6) = value6;

    // save
    OpenMesh::IO::Options options;
    bool ok = OpenMesh::IO::write_mesh(mesh,filename);
    EXPECT_TRUE(ok) << "Unable to write "<<filename;

    // load
    Mesh cmpMesh;

    cmpMesh.add_property(prop,propName);
    cmpMesh.property(prop).set_persistent(true);

    ok = OpenMesh::IO::read_mesh(cmpMesh,filename);
    EXPECT_TRUE(ok) << "Unable to read "<<filename;

    // compare
    EXPECT_EQ(4u  , cmpMesh.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(6u , cmpMesh.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(3u , cmpMesh.n_faces())    << "The number of loaded faces is not correct!";

    EXPECT_EQ(Mesh::Point(1.0,0.0,0.0) , cmpMesh.point(v1)) << "Wrong coordinates at vertex 0";
    EXPECT_EQ(Mesh::Point(0.0,1.0,0.0) , cmpMesh.point(v2)) << "Wrong coordinates at vertex 1";
    EXPECT_EQ(Mesh::Point(0.0,0.0,1.0) , cmpMesh.point(v3)) << "Wrong coordinates at vertex 2";

#ifdef TEST_DOUBLE_TRAITS
    // TODO: should it be possible to read and write double precision exactly?
    EXPECT_FLOAT_EQ(c[0] , cmpMesh.point(v4)[0]) << "Wrong coordinate 0 at vertex 4";
    EXPECT_FLOAT_EQ(c[1] , cmpMesh.point(v4)[1]) << "Wrong coordinate 1 at vertex 4";
    EXPECT_FLOAT_EQ(c[2] , cmpMesh.point(v4)[2]) << "Wrong coordinate 2 at vertex 4";
#else
    EXPECT_EQ(c , cmpMesh.point(v4)) << "Wrong coordinates at vertex 4";
#endif

    EXPECT_EQ(value1 , cmpMesh.property(prop,e1)) << "Wrong property at edge 0";
    EXPECT_EQ(value2 , cmpMesh.property(prop,e2)) << "Wrong property at edge 1";
    EXPECT_EQ(value3 , cmpMesh.property(prop,e3)) << "Wrong property at edge 2";
    EXPECT_EQ(value4 , cmpMesh.property(prop,e4)) << "Wrong property at edge 3";
    EXPECT_EQ(value5 , cmpMesh.property(prop,e5)) << "Wrong property at edge 4";
    EXPECT_EQ(value6 , cmpMesh.property(prop,e6)) << "Wrong property at edge 5";
    // The above only shows that the edge properties are stored in the same order which is not what we want if the edges are different

    // Check edge properties based on edges defined by from and to vertex
    for (auto eh : mesh.edges())
    {
      auto heh = mesh.halfedge_handle(eh, 0);
      auto from_vh = mesh.from_vertex_handle(heh);
      auto to_vh = mesh.to_vertex_handle(heh);

      // find corresponding halfedge in loaded mesh
      auto cmpHeh = cmpMesh.find_halfedge(from_vh, to_vh);
      auto cmpEh = cmpMesh.edge_handle(cmpHeh);

      EXPECT_EQ(mesh.property(prop, eh), cmpMesh.property(prop, cmpEh)) << "Wrong property at input edge " << eh.idx()
                                                                        << " corresponding to edge " << cmpEh.idx() << " in the loaded Mesh";
    }



    // cleanup
    remove(filename.c_str());

}

/*
 * Save and load simple mesh with status property
 */
TEST_F(OpenMeshReadWriteOM, WriteSplitTriangleStatusProperties) {

    Mesh mesh;

    mesh.request_vertex_status();
    mesh.request_edge_status();
    mesh.request_halfedge_status();
    mesh.request_face_status();

    const std::string filename = std::string("triangle-minimal-status.om");

    // generate data
    Mesh::VertexHandle v0 = mesh.add_vertex(Mesh::Point(1.0,0.0,0.0));
    Mesh::VertexHandle v1 = mesh.add_vertex(Mesh::Point(0.0,1.0,0.0));
    Mesh::VertexHandle v2 = mesh.add_vertex(Mesh::Point(0.0,0.0,1.0));
    auto fh0 = mesh.add_face(v0,v1,v2);
    auto c = mesh.calc_face_centroid(fh0);
    Mesh::VertexHandle v3 = mesh.add_vertex(c);
    mesh.split(fh0, v3);

    mesh.delete_vertex(v0);
    mesh.status(v1).set_selected(true);
    mesh.status(v2).set_feature(true);
    mesh.status(v3).set_tagged(true);
    mesh.status(v2).set_tagged2(true);

    std::vector<bool> vertex_deleted;
    std::vector<bool> vertex_selected;
    std::vector<bool> vertex_feature;
    std::vector<bool> vertex_tagged;
    std::vector<bool> vertex_tagged2;

    for (auto vh : mesh.all_vertices())
    {
      vertex_deleted.push_back(mesh.status(vh).deleted());
      vertex_selected.push_back(mesh.status(vh).selected());
      vertex_feature.push_back(mesh.status(vh).feature());
      vertex_tagged.push_back(mesh.status(vh).tagged());
      vertex_tagged2.push_back(mesh.status(vh).tagged2());
    }

    Mesh::EdgeHandle e1 = Mesh::EdgeHandle(0);
    Mesh::EdgeHandle e2 = Mesh::EdgeHandle(1);
    Mesh::EdgeHandle e3 = Mesh::EdgeHandle(2);
    Mesh::EdgeHandle e4 = Mesh::EdgeHandle(3);

    mesh.status(e1).set_selected(true);
    mesh.status(e2).set_feature(true);
    mesh.status(e3).set_tagged(true);
    mesh.status(e4).set_tagged2(true);

    std::vector<bool> edge_deleted;
    std::vector<bool> edge_selected;
    std::vector<bool> edge_feature;
    std::vector<bool> edge_tagged;
    std::vector<bool> edge_tagged2;

    for (auto eh : mesh.all_edges())
    {
      edge_deleted.push_back(mesh.status(eh).deleted());
      edge_selected.push_back(mesh.status(eh).selected());
      edge_feature.push_back(mesh.status(eh).feature());
      edge_tagged.push_back(mesh.status(eh).tagged());
      edge_tagged2.push_back(mesh.status(eh).tagged2());
    }


    Mesh::HalfedgeHandle he1 = Mesh::HalfedgeHandle(0);
    Mesh::HalfedgeHandle he2 = Mesh::HalfedgeHandle(3);
    Mesh::HalfedgeHandle he3 = Mesh::HalfedgeHandle(5);
    Mesh::HalfedgeHandle he4 = Mesh::HalfedgeHandle(1);

    mesh.status(he1).set_selected(true);
    mesh.status(he2).set_feature(true);
    mesh.status(he3).set_tagged(true);
    mesh.status(he4).set_tagged2(true);

    std::vector<bool> halfedge_deleted;
    std::vector<bool> halfedge_selected;
    std::vector<bool> halfedge_feature;
    std::vector<bool> halfedge_tagged;
    std::vector<bool> halfedge_tagged2;

    for (auto heh : mesh.all_halfedges())
    {
      halfedge_deleted.push_back(mesh.status(heh).deleted());
      halfedge_selected.push_back(mesh.status(heh).selected());
      halfedge_feature.push_back(mesh.status(heh).feature());
      halfedge_tagged.push_back(mesh.status(heh).tagged());
      halfedge_tagged2.push_back(mesh.status(heh).tagged2());
    }

    Mesh::FaceHandle f1 = Mesh::FaceHandle(0);
    Mesh::FaceHandle f2 = Mesh::FaceHandle(2);
    Mesh::FaceHandle f3 = Mesh::FaceHandle(1);
    Mesh::FaceHandle f4 = Mesh::FaceHandle(2);

    mesh.status(f1).set_selected(true);
    mesh.status(f2).set_feature(true);
    mesh.status(f3).set_tagged(true);
    mesh.status(f4).set_tagged2(true);

    std::vector<bool> face_deleted;
    std::vector<bool> face_selected;
    std::vector<bool> face_feature;
    std::vector<bool> face_tagged;
    std::vector<bool> face_tagged2;

    for (auto fh : mesh.all_faces())
    {
      face_deleted.push_back(mesh.status(fh).deleted());
      face_selected.push_back(mesh.status(fh).selected());
      face_feature.push_back(mesh.status(fh).feature());
      face_tagged.push_back(mesh.status(fh).tagged());
      face_tagged2.push_back(mesh.status(fh).tagged2());
    }

    // save
    OpenMesh::IO::Options options = OpenMesh::IO::Options::Status;
    bool ok = OpenMesh::IO::write_mesh(mesh,filename, options);
    EXPECT_TRUE(ok) << "Unable to write "<<filename;

    // load
    Mesh cmpMesh;

    cmpMesh.request_vertex_status();
    cmpMesh.request_edge_status();
    cmpMesh.request_halfedge_status();
    cmpMesh.request_face_status();

    ok = OpenMesh::IO::read_mesh(cmpMesh,filename, options);
    EXPECT_TRUE(ok) << "Unable to read "<<filename;

    // compare
    EXPECT_EQ(4u , cmpMesh.n_vertices()) << "The number of loaded vertices is not correct!";
    EXPECT_EQ(6u , cmpMesh.n_edges())    << "The number of loaded edges is not correct!";
    EXPECT_EQ(3u , cmpMesh.n_faces())    << "The number of loaded faces is not correct!";

    EXPECT_EQ(Mesh::Point(1.0,0.0,0.0) , cmpMesh.point(v0)) << "Wrong coordinates at vertex 0";
    EXPECT_EQ(Mesh::Point(0.0,1.0,0.0) , cmpMesh.point(v1)) << "Wrong coordinates at vertex 1";
    EXPECT_EQ(Mesh::Point(0.0,0.0,1.0) , cmpMesh.point(v2)) << "Wrong coordinates at vertex 2";

#ifdef TEST_DOUBLE_TRAITS
    // TODO: should it be possible to read and write double precision exactly?
    EXPECT_FLOAT_EQ(c[0] , cmpMesh.point(v3)[0]) << "Wrong coordinate 0 at vertex 3";
    EXPECT_FLOAT_EQ(c[1] , cmpMesh.point(v3)[1]) << "Wrong coordinate 1 at vertex 3";
    EXPECT_FLOAT_EQ(c[2] , cmpMesh.point(v3)[2]) << "Wrong coordinate 2 at vertex 3";
#else
    EXPECT_EQ(c , cmpMesh.point(v3)) << "Wrong coordinates at vertex 3";
#endif

    for (auto vh : cmpMesh.all_vertices())
    {
      EXPECT_EQ(cmpMesh.status(vh).deleted(),  vertex_deleted [vh.idx()]) << "Wrong deleted status at vertex " << vh.idx();
      EXPECT_EQ(cmpMesh.status(vh).selected(), vertex_selected[vh.idx()]) << "Wrong selected status at vertex " << vh.idx();
      EXPECT_EQ(cmpMesh.status(vh).feature(),  vertex_feature [vh.idx()]) << "Wrong feature status at vertex " << vh.idx();
      EXPECT_EQ(cmpMesh.status(vh).tagged(),   vertex_tagged  [vh.idx()]) << "Wrong tagged status at vertex " << vh.idx();
      EXPECT_EQ(cmpMesh.status(vh).tagged2(),  vertex_tagged2 [vh.idx()]) << "Wrong tagged2 status at vertex " << vh.idx();
    }

    for (auto eh : cmpMesh.all_edges())
    {
      EXPECT_EQ(cmpMesh.status(eh).deleted(),  edge_deleted [eh.idx()]) << "Wrong deleted status at edge " << eh.idx();
      EXPECT_EQ(cmpMesh.status(eh).selected(), edge_selected[eh.idx()]) << "Wrong selected status at edge " << eh.idx();
      EXPECT_EQ(cmpMesh.status(eh).feature(),  edge_feature [eh.idx()]) << "Wrong feature status at edge " << eh.idx();
      EXPECT_EQ(cmpMesh.status(eh).tagged(),   edge_tagged  [eh.idx()]) << "Wrong tagged status at edge " << eh.idx();
      EXPECT_EQ(cmpMesh.status(eh).tagged2(),  edge_tagged2 [eh.idx()]) << "Wrong tagged2 status at edge " << eh.idx();
    }

    for (auto heh : cmpMesh.all_halfedges())
    {
      EXPECT_EQ(cmpMesh.status(heh).deleted(),  halfedge_deleted [heh.idx()]) << "Wrong deleted status at halfedge " << heh.idx();
      EXPECT_EQ(cmpMesh.status(heh).selected(), halfedge_selected[heh.idx()]) << "Wrong selected status at halfedge " << heh.idx();
      EXPECT_EQ(cmpMesh.status(heh).feature(),  halfedge_feature [heh.idx()]) << "Wrong feature status at halfedge " << heh.idx();
      EXPECT_EQ(cmpMesh.status(heh).tagged(),   halfedge_tagged  [heh.idx()]) << "Wrong tagged status at halfedge " << heh.idx();
      EXPECT_EQ(cmpMesh.status(heh).tagged2(),  halfedge_tagged2 [heh.idx()]) << "Wrong tagged2 status at halfedge " << heh.idx();
    }

    for (auto fh : cmpMesh.all_faces())
    {
      EXPECT_EQ(cmpMesh.status(fh).deleted(),  face_deleted [fh.idx()]) << "Wrong deleted status at face " << fh.idx();
      EXPECT_EQ(cmpMesh.status(fh).selected(), face_selected[fh.idx()]) << "Wrong selected status at face " << fh.idx();
      EXPECT_EQ(cmpMesh.status(fh).feature(),  face_feature [fh.idx()]) << "Wrong feature status at face " << fh.idx();
      EXPECT_EQ(cmpMesh.status(fh).tagged(),   face_tagged  [fh.idx()]) << "Wrong tagged status at face " << fh.idx();
      EXPECT_EQ(cmpMesh.status(fh).tagged2(),  face_tagged2 [fh.idx()]) << "Wrong tagged2 status at face " << fh.idx();
    }


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

    float va1ue1 = 3.1f;

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


/*
 * Save and load simple mesh with vertex status
 */
TEST_F(OpenMeshReadWriteOM, WriteReadStatusPropertyVertexOnly) {

  //read file
  Mesh mesh;
  auto vh0 = mesh.add_vertex(Mesh::Point(0,0,0));
  auto vh1 = mesh.add_vertex(Mesh::Point(1,0,0));
  auto vh2 = mesh.add_vertex(Mesh::Point(0,1,0));
  mesh.add_face(vh0, vh1, vh2);

  mesh.request_vertex_status();

  mesh.status(vh0).set_selected(true);
  mesh.status(vh1).set_feature(true);
  mesh.status(vh2).set_tagged(true);
  mesh.status(vh0).set_locked(true);
  mesh.status(vh1).set_deleted(true);
  mesh.status(vh2).set_hidden(true);
  mesh.status(vh0).set_fixed_nonmanifold(true);


  std::string filename_without_status = "no_vertex_status_test.om";
  std::string filename_with_status = "vertex_status_test.om";

  OpenMesh::IO::Options opt_with_status = OpenMesh::IO::Options::Status;
  OpenMesh::IO::write_mesh(mesh, filename_without_status);
  OpenMesh::IO::write_mesh(mesh, filename_with_status, opt_with_status);

  // Load no status from file with status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though they should not have been loaded";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though file should not have a stored status";
  }

  // Load status from file with status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status, opt_with_status);

    EXPECT_TRUE (loaded_mesh.has_vertex_status())   << "Mesh has no vertex status even though they should have been loaded";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though file should not have a stored status";

    if (loaded_mesh.has_vertex_status())
    {
      for (auto vh : mesh.vertices())
      {
        EXPECT_EQ(mesh.status(vh).bits(), loaded_mesh.status(vh).bits());
      }
    }
  }

  // Load no status from file with status
  {
    Mesh loaded_mesh;
    loaded_mesh.request_vertex_status();
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status);

    EXPECT_TRUE (loaded_mesh.has_vertex_status())   << "Mesh vertex status was removed by reading";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though file should not have a stored status";

    for (auto vh : loaded_mesh.vertices())
    {
      EXPECT_EQ(loaded_mesh.status(vh).bits(), 0u) << "Vertex status was modified even though it should not have been loaded";
    }
  }

  // Try to load status from file without status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_without_status, opt_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though they file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though they file should not have a stored status";
  }
}


/*
 * Save and load simple mesh with halfedge status
 */
TEST_F(OpenMeshReadWriteOM, WriteReadStatusPropertyHalfedgeOnly) {

  //read file
  Mesh mesh;
  auto vh0 = mesh.add_vertex(Mesh::Point(0,0,0));
  auto vh1 = mesh.add_vertex(Mesh::Point(1,0,0));
  auto vh2 = mesh.add_vertex(Mesh::Point(0,1,0));
  mesh.add_face(vh0, vh1, vh2);

  mesh.request_halfedge_status();

  auto heh0 = OpenMesh::HalfedgeHandle(0);
  auto heh1 = OpenMesh::HalfedgeHandle(1);
  auto heh2 = OpenMesh::HalfedgeHandle(2);
  auto heh3 = OpenMesh::HalfedgeHandle(3);
  auto heh4 = OpenMesh::HalfedgeHandle(4);
  auto heh5 = OpenMesh::HalfedgeHandle(5);

  mesh.status(heh0).set_selected(true);
  mesh.status(heh1).set_feature(true);
  mesh.status(heh2).set_tagged(true);
  mesh.status(heh3).set_locked(true);
  mesh.status(heh4).set_deleted(true);
  mesh.status(heh5).set_hidden(true);
  mesh.status(heh0).set_fixed_nonmanifold(true);

  std::string filename_without_status = "no_halfedge_status_test.om";
  std::string filename_with_status = "edge_halfstatus_test.om";

  OpenMesh::IO::Options opt_with_status = OpenMesh::IO::Options::Status;
  OpenMesh::IO::write_mesh(mesh, filename_without_status);
  OpenMesh::IO::write_mesh(mesh, filename_with_status, opt_with_status);

  // Load no status from file with status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though they should not have been loaded";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though file should not have a stored status";
  }

  // Load status from file with status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status, opt_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though they should have been loaded";
    EXPECT_TRUE(loaded_mesh.has_halfedge_status())  << "Mesh has no halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though file should not have a stored status";

    if (loaded_mesh.has_halfedge_status())
    {
      for (auto heh : mesh.halfedges())
      {
        EXPECT_EQ(mesh.status(heh).bits(), loaded_mesh.status(heh).bits());
      }
    }
  }

  // Load no status from file with status
  {
    Mesh loaded_mesh;
    loaded_mesh.request_halfedge_status();
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though file should not have a stored status";
    EXPECT_TRUE (loaded_mesh.has_halfedge_status()) << "Mesh halfedge status was removed by reading";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though file should not have a stored status";

    for (auto heh : loaded_mesh.halfedges())
    {
      EXPECT_EQ(loaded_mesh.status(heh).bits(), 0u) << "Edge status was modified even though it should not have been loaded";
    }
  }

  // Try to load status from file without status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_without_status, opt_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though they file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though they file should not have a stored status";
  }
}


/*
 * Save and load simple mesh with edge status
 */
TEST_F(OpenMeshReadWriteOM, WriteReadStatusPropertyEdgeOnly) {

  //read file
  Mesh mesh;
  auto vh0 = mesh.add_vertex(Mesh::Point(0,0,0));
  auto vh1 = mesh.add_vertex(Mesh::Point(1,0,0));
  auto vh2 = mesh.add_vertex(Mesh::Point(0,1,0));
  mesh.add_face(vh0, vh1, vh2);

  mesh.request_edge_status();

  auto eh0 = OpenMesh::EdgeHandle(0);
  auto eh1 = OpenMesh::EdgeHandle(1);
  auto eh2 = OpenMesh::EdgeHandle(2);

  mesh.status(eh0).set_selected(true);
  mesh.status(eh1).set_feature(true);
  mesh.status(eh2).set_tagged(true);
  mesh.status(eh0).set_locked(true);
  mesh.status(eh1).set_deleted(true);
  mesh.status(eh2).set_hidden(true);
  mesh.status(eh0).set_fixed_nonmanifold(true);

  std::string filename_without_status = "no_edge_status_test.om";
  std::string filename_with_status = "edge_status_test.om";

  OpenMesh::IO::Options opt_with_status = OpenMesh::IO::Options::Status;
  OpenMesh::IO::write_mesh(mesh, filename_without_status);
  OpenMesh::IO::write_mesh(mesh, filename_with_status, opt_with_status);

  // Load no status from file with status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though they should not have been loaded";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though file should not have a stored status";
  }

  // Load status from file with status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status, opt_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though they should have been loaded";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_TRUE(loaded_mesh.has_edge_status())      << "Mesh has no edge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though file should not have a stored status";

    if (loaded_mesh.has_edge_status())
    {
      for (auto eh : mesh.edges())
      {
        EXPECT_EQ(mesh.status(eh).bits(), loaded_mesh.status(eh).bits());
      }
    }
  }

  // Load no status from file with status
  {
    Mesh loaded_mesh;
    loaded_mesh.request_edge_status();
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_TRUE (loaded_mesh.has_edge_status())     << "Mesh edge status was removed by reading";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though file should not have a stored status";

    for (auto eh : loaded_mesh.edges())
    {
      EXPECT_EQ(loaded_mesh.status(eh).bits(), 0u) << "Edge status was modified even though it should not have been loaded";
    }
  }

  // Try to load status from file without status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_without_status, opt_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though they file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though they file should not have a stored status";
  }
}


/*
 * Save and load simple mesh with face status
 */
TEST_F(OpenMeshReadWriteOM, WriteReadStatusPropertyFaceOnly) {

  //read file
  Mesh mesh;
  auto vh0 = mesh.add_vertex(Mesh::Point(0,0,0));
  auto vh1 = mesh.add_vertex(Mesh::Point(1,0,0));
  auto vh2 = mesh.add_vertex(Mesh::Point(0,1,0));
  auto vh3 = mesh.add_vertex(Mesh::Point(1,1,0));
  auto fh0 = mesh.add_face(vh0, vh1, vh2);
  auto fh1 = mesh.add_face(vh2, vh1, vh3);

  mesh.request_face_status();

  mesh.status(fh0).set_selected(true);
  mesh.status(fh1).set_feature(true);
  mesh.status(fh0).set_tagged(true);
  mesh.status(fh1).set_locked(true);
  mesh.status(fh0).set_deleted(true);
  mesh.status(fh1).set_hidden(true);
  mesh.status(fh0).set_fixed_nonmanifold(true);

  std::string filename_without_status = "no_face_status_test.om";
  std::string filename_with_status = "face_status_test.om";

  OpenMesh::IO::Options opt_with_status = OpenMesh::IO::Options::Status;
  OpenMesh::IO::write_mesh(mesh, filename_without_status);
  OpenMesh::IO::write_mesh(mesh, filename_with_status, opt_with_status);

  // Load no status from file with status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though they should not have been loaded";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though file should not have a stored status";
  }

  // Load status from file with status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status, opt_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though they should have been loaded";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though file should not have a stored status";
    EXPECT_TRUE(loaded_mesh.has_face_status())      << "Mesh has no face status even though file should not have a stored status";

    if (loaded_mesh.has_face_status())
    {
      for (auto fh : mesh.faces())
      {
        EXPECT_EQ(mesh.status(fh).bits(), loaded_mesh.status(fh).bits());
      }
    }
  }

  // Load no status from file with status
  {
    Mesh loaded_mesh;
    loaded_mesh.request_face_status();
    OpenMesh::IO::read_mesh(loaded_mesh, filename_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edgestatus even though file should not have a stored status";
    EXPECT_TRUE (loaded_mesh.has_face_status())     << "Mesh face status was removed by reading";

    for (auto fh : loaded_mesh.faces())
    {
      EXPECT_EQ(loaded_mesh.status(fh).bits(), 0u) << "Edge status was modified even though it should not have been loaded";
    }
  }

  // Try to load status from file without status
  {
    Mesh loaded_mesh;
    OpenMesh::IO::read_mesh(loaded_mesh, filename_without_status, opt_with_status);

    EXPECT_FALSE(loaded_mesh.has_vertex_status())   << "Mesh has vertex status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_halfedge_status()) << "Mesh has halfedge status even though file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_edge_status())     << "Mesh has edge status even though they file should not have a stored status";
    EXPECT_FALSE(loaded_mesh.has_face_status())     << "Mesh has face status even though they file should not have a stored status";
  }
}


/*
 * Just load a triangle mesh from an om file of version 1.2
 */
TEST_F(OpenMeshReadWriteOM, LoadTriangleMeshVersion_1_2) {

  mesh_.clear();

  std::string file_name = "cube_tri_version_1_2.om";

  bool ok = OpenMesh::IO::read_mesh(mesh_, file_name);

  EXPECT_TRUE(ok) << file_name;

  EXPECT_EQ(8u  , mesh_.n_vertices())  << "The number of loaded vertices is not correct!";
  EXPECT_EQ(18u , mesh_.n_edges())     << "The number of loaded edges is not correct!";
  EXPECT_EQ(12u , mesh_.n_faces())     << "The number of loaded faces is not correct!";
  EXPECT_EQ(36u , mesh_.n_halfedges()) << "The number of loaded halfedges is not correct!";
}


/*
 * Just load a polyhedral mesh from an om file of version 1.2
 */
TEST_F(OpenMeshReadWriteOM, LoadPolyMeshVersion_1_2) {

  PolyMesh mesh;

  std::string file_name = "cube_poly_version_1_2.om";

  bool ok = OpenMesh::IO::read_mesh(mesh, file_name);

  EXPECT_TRUE(ok) << file_name;

  EXPECT_EQ(8u  , mesh.n_vertices())  << "The number of loaded vertices is not correct!";
  EXPECT_EQ(12u , mesh.n_edges())      << "The number of loaded edges is not correct!";
  EXPECT_EQ(6u  , mesh.n_faces())      << "The number of loaded faces is not correct!";
  EXPECT_EQ(24u , mesh.n_halfedges())  << "The number of loaded halfedges is not correct!";
}


/*
 * Just load a triangle mesh from an om file of version 2.0
 */
TEST_F(OpenMeshReadWriteOM, LoadTriangleMeshVersion_2_0) {

  mesh_.clear();

  std::string file_name = "cube_tri_version_2_0.om";

  bool ok = OpenMesh::IO::read_mesh(mesh_, file_name);

  EXPECT_TRUE(ok) << file_name;

  EXPECT_EQ(8u  , mesh_.n_vertices())  << "The number of loaded vertices is not correct!";
  EXPECT_EQ(18u , mesh_.n_edges())     << "The number of loaded edges is not correct!";
  EXPECT_EQ(12u , mesh_.n_faces())     << "The number of loaded faces is not correct!";
  EXPECT_EQ(36u , mesh_.n_halfedges()) << "The number of loaded halfedges is not correct!";
}


/*
 * Just load a polyhedral mesh from an om file of version 2.0
 */
TEST_F(OpenMeshReadWriteOM, LoadPolyMeshVersion_2_0) {

  PolyMesh mesh;

  std::string file_name = "cube_poly_version_2_0.om";

  bool ok = OpenMesh::IO::read_mesh(mesh, file_name);

  EXPECT_TRUE(ok) << file_name;

  EXPECT_EQ(8u  , mesh.n_vertices())  << "The number of loaded vertices is not correct!";
  EXPECT_EQ(12u , mesh.n_edges())      << "The number of loaded edges is not correct!";
  EXPECT_EQ(6u  , mesh.n_faces())      << "The number of loaded faces is not correct!";
  EXPECT_EQ(24u , mesh.n_halfedges())  << "The number of loaded halfedges is not correct!";
}


/*
 * Try to load mesh from om file with a version that is not yet supported
 */
TEST_F(OpenMeshReadWriteOM, LoadTriMeshVersion_7_5) {

  PolyMesh mesh;

  std::string file_name = "cube_tri_version_7_5.om";

  bool ok = OpenMesh::IO::read_mesh(mesh, file_name);

  EXPECT_FALSE(ok) << file_name;
}


/*
 * Try to write and load positions and normals that can only be represented by doubles
 */
TEST_F(OpenMeshReadWriteOM, WriteAndLoadDoubles) {

  typedef OpenMesh::PolyMesh_ArrayKernelT<OpenMesh::DefaultTraitsDouble> DoublePolyMesh;

  DoublePolyMesh mesh;
  mesh.request_vertex_normals();
  mesh.request_face_normals();

  std::vector<OpenMesh::VertexHandle> vertices;
  for (int i = 0; i < 3; ++i)
  {
    vertices.push_back(mesh.add_vertex(DoublePolyMesh::Point(1.0/3.0, std::numeric_limits<double>::min(), std::numeric_limits<double>::max())));
    mesh.set_normal(vertices.back(), DoublePolyMesh::Normal(1.0/3.0, std::numeric_limits<double>::min(), std::numeric_limits<double>::max()));
  }
  auto fh = mesh.add_face(vertices);
  mesh.set_normal(fh, DoublePolyMesh::Normal(1.0/3.0, std::numeric_limits<double>::min(), std::numeric_limits<double>::max()));

  std::string file_name = "doubles.om";

  OpenMesh::IO::Options opt = OpenMesh::IO::Options::VertexNormal | OpenMesh::IO::Options::FaceNormal;
  ASSERT_TRUE(OpenMesh::IO::write_mesh(mesh, file_name, opt)) << "Could not write file " << file_name;

  DoublePolyMesh mesh2;
  mesh2.request_vertex_normals();
  mesh2.request_face_normals();

  ASSERT_TRUE(OpenMesh::IO::read_mesh(mesh2, file_name, opt)) << "Could not read file " << file_name;

  EXPECT_EQ(mesh.point(OpenMesh::VertexHandle(0)), mesh2.point(OpenMesh::VertexHandle(0)));
  EXPECT_EQ(mesh.normal(OpenMesh::VertexHandle(0)), mesh2.normal(OpenMesh::VertexHandle(0)));
  EXPECT_EQ(mesh.normal(OpenMesh::FaceHandle(0)), mesh2.normal(OpenMesh::FaceHandle(0)));
}

}

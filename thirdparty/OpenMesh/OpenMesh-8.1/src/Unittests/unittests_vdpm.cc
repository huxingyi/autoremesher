#include <gtest/gtest.h>
#include <Unittests/unittests_common.hh>

#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>

#include <OpenMesh/Tools/VDPM/VHierarchy.hh>
#include <OpenMesh/Tools/VDPM/VHierarchyNode.hh>
#include <OpenMesh/Tools/VDPM/VHierarchyNodeIndex.hh>

namespace {

    class OpenMeshVDPM : public OpenMeshBase {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {
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
 *  2 tests, one is creating a pm file (cube1.pm) and the second test loads and analyze the readings.
 */

struct PMInfo
{
    Mesh::Point		p0;
    Mesh::VertexHandle	v0, v1, vl, vr;
};

struct VDPMTraits : public OpenMesh::DefaultTraits
{
    typedef OpenMesh::VDPM::VHierarchyNodeHandle VHierarchyNodeHandle;
    typedef OpenMesh::VDPM::VHierarchyNodeHandle VHierarchyNodeIndex;
    VertexTraits
    {
    public:

        VHierarchyNodeHandle vhierarchy_node_handle()
        {
            return node_handle_;
        }

        void set_vhierarchy_node_handle(VHierarchyNodeHandle _node_handle)
        {
            node_handle_ = _node_handle;
        }

        bool is_ancestor(const VHierarchyNodeIndex &_other)
        {
            return false;
        }

    private:

        VHierarchyNodeHandle  node_handle_;

    };


    HalfedgeTraits
    {
    public:

        VHierarchyNodeHandle
            vhierarchy_leaf_node_handle()
        {
            return  leaf_node_handle_;
        }

        void
            set_vhierarchy_leaf_node_handle(VHierarchyNodeHandle _leaf_node_handle)
        {
            leaf_node_handle_ = _leaf_node_handle;
        }

    private:

        VHierarchyNodeHandle  leaf_node_handle_;

    };

    VertexAttributes(OpenMesh::Attributes::Status |
        OpenMesh::Attributes::Normal);
    HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
    EdgeAttributes(OpenMesh::Attributes::Status);
    FaceAttributes(OpenMesh::Attributes::Status |
        OpenMesh::Attributes::Normal);

};

typedef OpenMesh::TriMesh_ArrayKernelT<VDPMTraits>  VDPMMesh;

struct LoadInfo
{
    unsigned n_base_vertices;
    unsigned n_base_faces;
    unsigned n_details;
    std::vector<PMInfo> pm_infos;
    OpenMesh::VDPM::VHierarchy vhierarchy;
    VDPMMesh mesh;
};

LoadInfo open_progresult_mesh(const std::string& _filename)
{
    VDPMMesh::Point           p;
    unsigned int          i, i0, i1, i2;
    unsigned int          v1, vl, vr;
    char                  c[10];
    VDPMMesh::VertexHandle          vertex_handle;
    OpenMesh::VDPM::VHierarchyNodeHandle  node_handle, lchild_handle, rchild_handle;
    OpenMesh::VDPM::VHierarchyNodeIndex   node_index;
    LoadInfo result;

    std::ifstream  ifs(_filename.c_str(), std::ios::binary);
    if (!ifs)
    {
        EXPECT_TRUE(false) << "Could not open file";
        return result;
    }

    //
    bool swap = OpenMesh::Endian::local() != OpenMesh::Endian::LSB;

    // read header
    ifs.read(c, 8); c[8] = '\0';
    if (std::string(c) != std::string("ProgMesh"))
    {
        EXPECT_TRUE(false) << "Wrong file format";
        return result;
    }

    OpenMesh::IO::restore(ifs, result.n_base_vertices, swap);
    OpenMesh::IO::restore(ifs, result.n_base_faces, swap);
    OpenMesh::IO::restore(ifs, result.n_details, swap);

    result.vhierarchy.set_num_roots(result.n_base_vertices);

    for (i = 0; i<result.n_base_vertices; ++i)
    {
        OpenMesh::IO::restore(ifs, p, swap);

        vertex_handle = result.mesh.add_vertex(p);
        node_index = result.vhierarchy.generate_node_index(i, 1);
        node_handle = result.vhierarchy.add_node();

        result.vhierarchy.node(node_handle).set_index(node_index);
        result.vhierarchy.node(node_handle).set_vertex_handle(vertex_handle);
        result.mesh.data(vertex_handle).set_vhierarchy_node_handle(node_handle);
    }

    for (i = 0; i<result.n_base_faces; ++i)
    {
        OpenMesh::IO::restore(ifs, i0, swap);
        OpenMesh::IO::restore(ifs, i1, swap);
        OpenMesh::IO::restore(ifs, i2, swap);
        result.mesh.add_face(result.mesh.vertex_handle(i0),
            result.mesh.vertex_handle(i1),
            result.mesh.vertex_handle(i2));
    }

    // load progressive detail
    for (i = 0; i<result.n_details; ++i)
    {
        OpenMesh::IO::restore(ifs, p, swap);
        OpenMesh::IO::restore(ifs, v1, swap);
        OpenMesh::IO::restore(ifs, vl, swap);
        OpenMesh::IO::restore(ifs, vr, swap);

        PMInfo pminfo;
        pminfo.p0 = OpenMesh::vector_cast<Mesh::Point>(p);
        pminfo.v0 = result.mesh.add_vertex(p);
        pminfo.v1 = Mesh::VertexHandle(v1);
        pminfo.vl = Mesh::VertexHandle(vl);
        pminfo.vr = Mesh::VertexHandle(vr);
        result.pm_infos.push_back(pminfo);

        node_handle = result.mesh.data(pminfo.v1).vhierarchy_node_handle();

        result.vhierarchy.make_children(node_handle);
        lchild_handle = result.vhierarchy.lchild_handle(node_handle);
        rchild_handle = result.vhierarchy.rchild_handle(node_handle);

        result.mesh.data(pminfo.v0).set_vhierarchy_node_handle(lchild_handle);
        result.mesh.data(pminfo.v1).set_vhierarchy_node_handle(rchild_handle);
        result.vhierarchy.node(lchild_handle).set_vertex_handle(pminfo.v0);
        result.vhierarchy.node(rchild_handle).set_vertex_handle(pminfo.v1);
    }

    ifs.close();


    // recover mapping between basemesh vertices to roots of vertex hierarchy
    for (i = 0; i<result.n_base_vertices; ++i)
    {
        node_handle = result.vhierarchy.root_handle(i);
        vertex_handle = result.vhierarchy.node(node_handle).vertex_handle();

        result.mesh.data(vertex_handle).set_vhierarchy_node_handle(node_handle);
    }

    return result;
}

/*
* Reads and checks the progressive mesh file
*/

TEST_F(OpenMeshVDPM, ReadPM)
{
    LoadInfo info = open_progresult_mesh("cube1.pm");

    EXPECT_EQ(7526u, info.mesh.n_vertices()) << "Vertices differ";
    EXPECT_EQ(6u, info.mesh.n_edges()) << "Edges differ";
    EXPECT_EQ(4u, info.mesh.n_faces()) << "Faces differ";
    EXPECT_EQ(7522u, info.n_details) << "Details differ";
}

/*
* Creates a progressive mesh
*/
TEST_F(OpenMeshVDPM, WriteReadPM)
{
    Mesh mesh;
    bool ok = OpenMesh::IO::read_mesh(mesh, "cube1.off");

    ASSERT_TRUE(ok);

    typedef OpenMesh::Decimater::DecimaterT< Mesh >  Decimater;
    typedef OpenMesh::Decimater::ModProgMeshT< Mesh >::Handle HModProg;
    typedef OpenMesh::Decimater::ModQuadricT< Mesh >::Handle HModQuadric;

    Decimater decimater(mesh);
    HModProg hModProg;
    HModQuadric hModQuadric;
    decimater.add(hModQuadric);
    decimater.add(hModProg);
    decimater.initialize();
    decimater.decimate(0);

    std::string filename = "vdpm_test_file.pm";

    EXPECT_TRUE(decimater.module(hModProg).write(filename)) << "Could not write PM file.";

    LoadInfo info = open_progresult_mesh(filename);
    EXPECT_EQ(7526u, info.mesh.n_vertices()) << "Vertices differ";
    EXPECT_EQ(6u, info.mesh.n_edges()) << "Edges differ";
    EXPECT_EQ(4u, info.mesh.n_faces()) << "Faces differ";
    EXPECT_EQ(7522u, info.n_details) << "Details differ";

    remove(filename.c_str());
}





}

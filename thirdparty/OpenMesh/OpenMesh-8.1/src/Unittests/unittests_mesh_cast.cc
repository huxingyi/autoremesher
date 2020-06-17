#include <iostream>

#include <gtest/gtest.h>

#include <Unittests/unittests_common.hh>


namespace {

class OpenMeshMeshCastTest: public testing::Test {
};

struct TriTraits1: public OpenMesh::DefaultTraits {
        typedef OpenMesh::Vec3d Point;
};
struct TriTraits2: public OpenMesh::DefaultTraits {
        typedef OpenMesh::Vec3d Point;
};

TEST_F(OpenMeshMeshCastTest, PerformCast) {
    OpenMesh::TriMesh_ArrayKernelT<TriTraits1> a;
    OpenMesh::TriMesh_ArrayKernelT<TriTraits2> &b =
            OpenMesh::mesh_cast<OpenMesh::TriMesh_ArrayKernelT<TriTraits2>&>(a);
    b.reserve(10,10,10);
    /*
    OpenMesh::TriMesh_ArrayKernelT < TriTraits2 > &b =
            OpenMesh::MeshCast<
                TriMesh_ArrayKernelT<TriTraits2>&,
                OpenMesh::TriMesh_ArrayKernelT<TriTraits1>&
            >::cast(a);
    */
}

}

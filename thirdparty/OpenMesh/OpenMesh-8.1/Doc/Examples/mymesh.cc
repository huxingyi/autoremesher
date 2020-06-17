#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>


// define traits
struct MyTraits : public OpenMesh::DefaultTraits
{
  // use double valued coordinates
  typedef OpenMesh::Vec3d Point;

  // use vertex normals and vertex colors
  VertexAttributes( OpenMesh::DefaultAttributer::Normal |
		    OpenMesh::DefaultAttributer::Color );

  // store the previous halfedge
  HalfedgeAttributes( OpenMesh::DefaultAttributer::PrevHalfedge );

  // use face normals
  FaceAttributes( OpenMesh::DefaultAttributer::Normal );

  // store a face handle for each vertex
  VertexTraits
  {
    typename Base::Refs::FaceHandle my_face_handle;
  };

};


// Select mesh type (TriMesh) and kernel (ArrayKernel)
// and define my personal mesh type (MyMesh)
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;

int main(int argc, char **argv)
{
  MyMesh mesh;

  // -------------------- Add dynamic data

  // for each vertex an extra double value
  OpenMesh::VPropHandleT< double > vprop_double;
  mesh.add_property( vprop_double );

  // for the mesh an extra string
  OpenMesh::MPropHandleT< string > mprop_string;
  mesh.add_property( mprop_string );

  // -------------------- do something

  ...;

}

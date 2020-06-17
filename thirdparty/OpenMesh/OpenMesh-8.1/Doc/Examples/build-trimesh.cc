template <class Traits>
struct TriMesh_ArrayKernel_GeneratorT
{
  typedef FinalMeshItemsT<ArrayItems, Traits, true>  MeshItems;
  typedef AttribKernelT<MeshItems>                   AttribKernel;
  typedef ArrayKernelT<AttribKernel, MeshItems>      MeshKernel;
  typedef TriMeshT<MeshKernel>                       Mesh;
};

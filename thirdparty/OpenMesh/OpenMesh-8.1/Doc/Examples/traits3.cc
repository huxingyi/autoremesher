struct MyTraits : public OpenMesh::DefaultTraits
{
  template <class Base, class Refs> struct VertexT : public Base
  {
    int some_additional_index;
  };
};

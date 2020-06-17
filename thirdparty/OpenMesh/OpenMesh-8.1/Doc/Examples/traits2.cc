struct MyTraits : public OpenMesh::DefaultTraits
{
  VertexTraits
  {
    int some_additional_index;
  };
};

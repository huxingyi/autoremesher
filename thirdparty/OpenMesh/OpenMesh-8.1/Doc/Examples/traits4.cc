struct MyTraits : public OpenMesh::DefaultTraits 
{
  VertexTraits
  {
    int some_additional_index;
    typename Base::Refs::FaceHandle my_face_handle;
  };
};

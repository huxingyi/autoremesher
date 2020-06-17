struct MyTraits : public OpenMesh::DefaultTraits 
{
  VertexAttributes( OpenMesh::Attributes::Normal |
		    OpenMesh::Attributes::Color );

  FaceAttributes( OpenMesh::Attributes::Normal );
};

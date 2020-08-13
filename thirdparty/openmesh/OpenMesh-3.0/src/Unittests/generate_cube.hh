#ifndef GENERATE_CUBE_HH
#define GENERATE_CUBE_HH

template <typename MeshType>
size_t generate_cube( MeshType& mesh )
{
  typedef typename MeshType::VertexHandle VertexHandle;
  typedef typename MeshType::Point Point;

  typename MeshType::VertexHandle vhandle[8];

  vhandle[0] = mesh.add_vertex(Point(-1, -1,  1));
  vhandle[1] = mesh.add_vertex(Point( 1, -1,  1));
  vhandle[2] = mesh.add_vertex(Point( 1,  1,  1));
  vhandle[3] = mesh.add_vertex(Point(-1,  1,  1));
  vhandle[4] = mesh.add_vertex(Point(-1, -1, -1));
  vhandle[5] = mesh.add_vertex(Point( 1, -1, -1));
  vhandle[6] = mesh.add_vertex(Point( 1,  1, -1));
  vhandle[7] = mesh.add_vertex(Point(-1,  1, -1));

  // generate (quadrilateral) faces

  std::vector< VertexHandle >  face_vhandles;

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[4]);
  mesh.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[5]);
  mesh.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[6]);
  mesh.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[7]);
  mesh.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[4]);
  mesh.add_face(face_vhandles);

  return mesh.n_vertices();
}

#endif

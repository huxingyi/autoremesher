MyMesh mesh;

// (linearly) iterate over all vertices
for (MyMesh::VertexIter v_it=mesh.vertices_sbegin(); v_it!=mesh.vertices_end(); ++v_it)
{
  // circulate around the current vertex
  for (MyMesh::VertexVertexIter vv_it=mesh.vv_iter(*v_it); vv_it.is_valid(); ++vv_it)
  {
    // do something with e.g. mesh.point(*vv_it)
  }
}

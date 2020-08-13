MyMesh mesh;

// iterate over all vertices
for (MyMesh::VertexIter v_it=mesh.vertices_begin(); v_it!=mesh.vertices_end(); ++v_it) 
   ...; // do something with *v_it, v_it->, or *v_it

// iterate over all halfedges
for (MyMesh::HalfedgeIter h_it=mesh.halfedges_begin(); h_it!=mesh.halfedges_end(); ++h_it) 
   ...; // do something with *h_it, h_it->, or *h_it

// iterate over all edges
for (MyMesh::EdgeIter e_it=mesh.edges_begin(); e_it!=mesh.edges_end(); ++e_it) 
   ...; // do something with *e_it, e_it->, or *e_it

// iterator over all faces
for (MyMesh::FaceIter f_it=mesh.faces_begin(); f_it!=mesh.faces_end(); ++f_it) 
   ...; // do something with *f_it, f_it->, or *f_it


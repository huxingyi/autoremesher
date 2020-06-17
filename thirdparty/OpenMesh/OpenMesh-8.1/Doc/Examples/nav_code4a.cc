// Get the halfedge handle of i.e. the halfedge
// that is associated to the first vertex
// of our set of vertices
PolyMesh::HalfedgeHandle heh = mesh.halfedge_handle(*(mesh.vertices_begin()));

// Now get the handle of its opposing halfedge
PolyMesh::HalfedgeHandle opposite_heh = mesh.opposite_halfedge_handle(heh);

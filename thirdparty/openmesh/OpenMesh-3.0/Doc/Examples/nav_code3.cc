[...]

// Get some vertex handle
PolyMesh::VertexHandle v = ...;

for(PolyMesh::VertexIHalfedgeIter vih_it = mesh.vih_iter(v); vih_it; ++vih_it) {
	// Iterate over all incoming halfedges...
}

for(PolyMesh::VertexOHalfedgeIter voh_it = mesh.voh_iter(v); voh_it; ++voh_it) {
	// Iterate over all outgoing halfedges...
}

[...]

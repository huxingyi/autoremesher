[...]

TriMesh::HalfedgeHandle heh, heh_init;

// Get the halfedge handle assigned to vertex[0]
heh = heh_init = mesh.halfedge_handle(vertex[0].handle());

// heh now holds the handle to the initial halfedge.
// We now get further on the boundary by requesting
// the next halfedge adjacent to the vertex heh
// points to...
heh = mesh.next_halfedge_handle(heh);

// We can do this as often as we want:
while(heh != heh_init) {
	heh = mesh.next_halfedge_handle(heh);
}

[...]

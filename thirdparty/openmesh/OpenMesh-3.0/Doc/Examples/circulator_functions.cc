/**************************************************
 * Vertex circulators
 **************************************************/

// Get the vertex-vertex circulator (1-ring) of vertex _vh
VertexVertexIter OpenMesh::PolyConnectivity::vv_iter (VertexHandle _vh);

// Get the vertex-incoming halfedges circulator of vertex _vh
VertexIHalfedgeIter OpenMesh::PolyConnectivity::vih_iter (VertexHandle _vh);

// Get the vertex-outgoing halfedges circulator of vertex _vh
VertexOHalfedgeIter OpenMesh::PolyConnectivity::voh_iter (VertexHandle _vh);

// Get the vertex-edge circulator of vertex _vh
VertexEdgeIter OpenMesh::PolyConnectivity::ve_iter (VertexHandle _vh);

// Get the vertex-face circulator of vertex _vh
VertexFaceIter OpenMesh::PolyConnectivity::vf_iter (VertexHandle _vh);

/**************************************************
 * Face circulators
 **************************************************/

// Get the face-vertex circulator of face _fh
FaceVertexIter OpenMesh::PolyConnectivity::fv_iter (FaceHandle _fh);

// Get the face-halfedge circulator of face _fh
FaceHalfedgeIter OpenMesh::PolyConnectivity::fh_iter (FaceHandle _fh);

// Get the face-edge circulator of face _fh
FaceEdgeIter OpenMesh::PolyConnectivity::fe_iter (FaceHandle _fh);

// Get the face-face circulator of face _fh
FaceFaceIter OpenMesh::PolyConnectivity::ff_iter (FaceHandle _fh);

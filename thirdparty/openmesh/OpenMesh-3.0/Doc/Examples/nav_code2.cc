// Test if a halfedge lies at a boundary (is not adjacent to a face)
bool is_boundary (HalfedgeHandle _heh) const

// Test if an edge lies at a boundary
bool is_boundary (EdgeHandle _eh) const

// Test if a vertex is adjacent to a boundary edge
bool is_boundary (VertexHandle _vh) const

// Test if a face has at least one adjacent boundary edge.
// If _check_vertex=true, this function also tests if at least one
// of the adjacent vertices is a boundary vertex
bool is_boundary (FaceHandle _fh, bool _check_vertex=false) const

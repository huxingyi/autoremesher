## HIGH PRIORITY

### Develop a helper to build the list of Ghost elements
- Read and allocate the required ElementGID, then build the list of elements that do not belong to the local domain and return it to the caller.

### Distributed parallel write
- Open a mesh file in writing mode but only create the skeleton of the mesh structure to enable further concurrent write access.
- Open an existing mesh file in writing mode and enable concurrent block write thanks to the existing structure.

## STANDARD PRIORITY

### Handle arbitrary degree high-order elements
- Setup a keyword to store Pk elements.
- Give along a routine to convert to and from well-known high-order numberings.

### Handle arbitrary degree polygons and polyhedra
- Add a helper that cuts a polyhedron through a plane and generates the intersection's triangulated mesh in an STL-like format.

### Solution fields comments
- Add a procedure that would search for a string among comments.
- Input:  keyword name, physical property, free comment, wildcards.
- Output: list of solutions keywords and particular field number.

### Convert HO examples to Fortran
- test_libmeshb_HO.c
- test_libmeshb_p2_sol.c

### Add F77 API to GmfSetHONodesOrdering
An easy one.

### Add IHOSol* + DHOSol* for each element kinds,
for example
"IHOSolAtVertices",                           "i", "ii" // ii = degree + index in DSol
"DHOSolAtVertices",                           "i", "hr" // High Order solution

### Topological operations
-Add a helper to build the list of inner or surface triangles from tetrahedra
-Add a helper to build the list of unique edges from tetrahedra
-Add a helper to get the face neighborhood between tets and triangles
-Add a helper to get an edge's shell of triangles or tets

### Documentation

## DONE

- Indirect storage through a renumbering table for parallel reading.
- Keyword documentation updated.
- High-Order multiple solutions per element for DG.
- Added ByteFlow keyword to store an arbitrary sized byte array.
- Added two examples to read and write EGADS CAD models stores as byte flows.
- Give a way to describe the node numbering.
- Setup a polygon keyword that provides an arbitrary degree and number of nodes.
- Setup a polyhedron keyword that provides an arbitrary degree and number of oriented polygons.
- Setup a boundary polygon keyword that lists the boundary faces among the ensemble of volume polygons.
- Create a separate helper file to be optionally compiled along the libMeshb.
- Added a helper to allocate and read the boundary polygons.
- Added a helper to allocate and read the polyhedra and inner polygons.
- Added a helper that returns a polygon's degree and nodes list.
- Added a helper that returns a polyhedron's degree and face indices list.
- Write the helper's documentation.
- Added an example that converts and fuses all surfaces and volume elements to polygons and polyhedra.
- Added a helper to tesselate a polygon into a set of triangles made of nodes and edge visibility flags.
- Added a set of keywords for each kind of element that stores the number of vertices and their barycentric coordinates.
- Added a section about the AIO mode (Asynchronous Input Output).
- Added a helper that evaluates the quality of a mesh numbering in terms of efficient cache reuse and inner concurrency.

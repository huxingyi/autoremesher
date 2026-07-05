## libMeshb helper functions

The source file **libMeshb7\_helpers.c** is a collection additional functions that you may include in your code. These functions are of higher level than the libMeshb which is a low level I/O library. Consequently, they may not be of interest to everyone and are not part of the library itself.

This higher-level code offers some data structures that store multiple fields (files, tables, information) as well as a high-level procedure to perform on these structures (open, allocate and read a whole mesh file with a single call for example).

You can either include the C file in your project or pick-up some functions and structures that suit your needs.

### Define the size of your integers and real numbers
Two data kinds are defined to easily switch from 32-bit and 64-bit integers and reals. This convention is used throughout the helper procedures.

**itg** should be used to define all integers instead of using **int** or **long**, its default size is 32 bits (int32\_t) and you only need to add the compiling flag **-DINT64** to switch all itg to 64-bit (int64\_t).

**fpn** should be used to define all reals instead of using **float** or **double**, its default size is 64 bits (double) and you only need to add the compiling flag **-DREAL32** to switch all itg to 32-bit (float).

### Polyhedral mesh structure
To easily handle arbitrary degree polygons and polyhedra, a new set of keywords have been added:
- GmfBoundaryPolygonHeaders
- GmfBoundaryPolygonVertices
- GmfInnerPolygonHeaders
- GmfInnerPolygonVertices
- GmfPolyhedraHeaders
- GmfPolyhedraFaces

All the tables pointers and sizes can be conveniently stored in this dedicated structure.
```C++
typedef struct
{
   int64_t  MshIdx;
   itg      NmbBndHdr, (*BndHdrTab)[2], NmbBndVer, *BndVerTab;
   itg      NmbInrHdr, (*InrHdrTab)[2], NmbInrVer, *InrVerTab;
   itg      NmbVolHdr, (*VolHdrTab)[2], NmbVolFac, *VolFacTab;
}PolMshSct;
```

- MshIdx stores the mesh file index this structure if associated with.

- NmbBndHdr: number of lines in the GmfBoundaryPolygonHeaders keyword (number of boundary polygons).
- BndHdrTab: table of polygon headers that give the index of the polygon's vertices table in BndVerTab and a material reference.
- NmbBndVer: total size of compounded boundary polygons vertices (GmfBoundaryPolygonVertices).
- BndVerTab: table of vertex indices of all boundary polygons.

- NmbInrHdr: number of lines in the GmfInnerPolygonHeaders keyword (number of inner polygons referenced by polyhedra).
- InrHdrTab: table of polygon headers that give the index of the polygon's vertices table in InrVerTab and a material reference.
- NmbInrVer: total size of compounded inner polygons vertices (GmfInnerPolygonVertices).
- InrVerTab: table of vertex indices of all inner polygons.

- NmbVolHdr: number of lines in the GmfPolyhedraHeaders keyword (number of polyhedra).
- VolHdrTab: table of polyhedra headers that give the index of the element face index table in VolFacTab and a material reference.
- NmbVolFac: total size of compounded polyhedra faces (GmfPolyhedraFaces).
- VolFacTab: table of polygons indices of all polyhedra.

### GmfAllocatePolyghedralStructure
Synopsis: this function only allocate memories but does not read the data from the mesh file.

- input: the index of an already opened mesh file that contains polygons or polyhedra.
- output: a PolMshSct structure set up with the table sizes and pointers to allocated memory.

### GmfFreePolyghedralStructure
Synopsis: free all tables and the structure itself.

- input: a pointer to an allocated PolMshSct structure.

### GmfReadBoundaryPolygons
Synopsis: reads the two keywords' fields, GmfBoundaryPolygonHeaders and GmfBoundaryPolygonVertices, into the polyhedra structure's tables.

- input: a pointer to an allocated PolMshSct structure.
- output: 0 in case of failure and 1 otherwise.

### GmfReadPolyhedra
Synopsis: reads the four keywords' fields, GmfInnerPolygonHeaders, GmfInnerPolygonVertices, GmfPolyhedraHeaders and GmfPolyhedraFaces, into the polyhedra structure's tables.

- input: a pointer to an allocated PolMshSct structure.
- output: 0 in case of failure and 1 otherwise.

### GmfGetBoundaryPolygon
Synopsis: extract a polygon's vertex list from the main global table and copy them to your local table.

- input: a pointer to an allocated PolMshSct structure, the index of a boundary polygonal face, a pointer to a table of **itg** that you have previously allocated with at least 256 entries.
- output: the polygon's number of vertices is the function's returned value (0 in case of failure) and the provided table is filled with the polygons' vertex indices.

### GmfGetInnerPolygon
Same procedure as above but for inner polygons only.

### GmfGetPolyhedron
Synopsis: extract a polyhedron's face list from the main global table and copy them to your local table.

- input: a pointer to an allocated PolMshSct structure, the index of a polyhedral element, a pointer to a table of **itg** that you have previously allocated with at least 256 entries.
- output: the polyhedron's number of faces is the function's returned value (0 in case of failure) and the provided table is filled with the polyhedron's face indices.

### GmlEvaluateNumbering
Synopsis: Evaluate the quality of a mesh elements and nodes numbering. Usually, Delaunay meshes are quite bad (quality < 50) and need renumbering, octree or advancing front meshes are tolerable (from 50 to 90), and Hilbert renumbered meshes have a quality greater than 90.

-input: the number of elements.
-input: the number of nodes that make-up this kind of element (edge = 2, tetrahedron = 4).
-input pointer to the first node of the first element.
-input pointer to the first node of the last element.
-output: a quality factor that ranges from 0 (bad) to 100 (perfect).

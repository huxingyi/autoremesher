[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## libMeshb version 7.62
A library to handle the *.meshb file format.

## Overview
The Gamma Mesh Format (*GMF*) and the associated library **libMeshb** provide programers of simulation and meshing software with an easy way to store their meshes and physical solutions.  
The *GMF* features more than 200 kinds of data types, like vertex, polyhedron, normal vector or vector solution field.  
The **libMeshb** provides a convenient way to move data between those files, via keyword tags, and the user's own structures.  
Transparent handling of ASCII & binary files.  
Transparent handling of little & big endian files.  
Optional ultra fast asynchronous and low level transfers.
Can call user's own pre and post processing routines in a separate thread while accessing a file.

### Prerequisites for *Linux* or *macOS*
- Install [CMake](https://cmake.org/files/v3.7/cmake-3.7.2-win64-x64.msi)
- A valid C99 compiler
- Open a shell window

### Prerequisites for *Windows*
- You first need to install [CMake](https://cmake.org/files/v3.7/cmake-3.7.2-win64-x64.msi). Do not forget to choose "add cmake to the path for all users", from the install panel.
- Then you need a valid C compiler like the free [Visual Studio Community 2019](https://www.visualstudio.com/vs/visual-studio-express/)
- Open the x64 Native Tools Command Prompt for VS (or x86 if you need to build a 32-bit version)

### Build commands for all platforms
- unarchive the ZIP file
- `cd libMeshb-master`
- `mkdir build`
- `cd build`
- `cmake ..`
- `cmake --build . --target install`

## Usage
The **libMeshb** library is written in *ANSI C*.  
It is made of a single C file and a header file to be compiled and linked alongside the calling program.  
It may be used in C, C++, F77 and F90 programs (Fortran 77 and 90 APIs are provided).  
Tested on *Linux*, *macOS*, and *Windows 7->10*.

Reading a mesh file is fairly easy:

```C++
int64_t LibIdx;
int ver, dim, NmbVer, NmbTri, (*Nodes)[4], *Domains;
float (*Coords)[3];

// Open the mesh file for reading
LibIdx = GmfOpenMesh( "triangles.meshb", GmfRead, &ver, &dim );

// Get the number of vertices and triangles
NmbVer = GmfStatKwd( LibIdx, GmfVertices  );
NmbTri = GmfStatKwd( LibIdx, GmfTriangles );

// Allocate memory accordingly
Nodes   = malloc( NmbTri * 4 * sizeof(int)   );
Coords  = malloc( NmbVer * 3 * sizeof(float) );
Domains = malloc( NmbVer     * sizeof(int)   );

// Move the file pointer to the vertices keyword
GmfGotoKwd( LibIdx, GmfVertices );

// Read each line of vertex data into your own data structures
for(i=0;i<NmbVer;i++)
  GmfGetLin( LibIdx, GmfVertices, &Coords[i][0], &Coords[i][1], &Coords[i][2], &Domains[i] );

// Move the file pointer to the triangles keyword
GmfGotoKwd( LibIdx, GmfTriangles );

// Read each line of triangle data into your own data structures
for(i=0;i<NmbTri;i++)
  GmfGetLin( LibIdx, GmfTriangles, &Nodes[i][0], &Nodes[i][1], &tNodest[i][2], &Nodes[i][3] );

// Close the mesh file !
GmfCloseMesh( LibIdx );
```

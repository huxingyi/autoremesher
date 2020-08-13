#include <iostream>
#include <vector>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
// -------------------- 
#include "smooth_algo.hh"


// ----------------------------------------------------------------------------

#ifndef DOXY_IGNORE_THIS

struct MyTraits : public OpenMesh::DefaultTraits
{
  HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
};

#endif

typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;


// ----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  MyMesh  mesh;


  // check command line options
  if (argc != 4) 
  {
    std::cerr << "Usage:  " << argv[0] << " #iterations  infile  outfile\n";
    return 1;
  }


  // read mesh from stdin
  if ( ! OpenMesh::IO::read_mesh(mesh, argv[2]) )
  {
     std::cerr << "Error: Cannot read mesh from " << argv[2] << std::endl;
     return 1;
  }


  // smoothing mesh argv[1] times
  SmootherT<MyMesh> smoother(mesh);
  smoother.smooth(atoi(argv[1]));


  // write mesh to stdout
  if ( ! OpenMesh::IO::write_mesh(mesh, argv[3]) )
  {
    std::cerr << "Error: cannot write mesh to " << argv[3] << std::endl;
    return 1;
  }
  return 0;
}

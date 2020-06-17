#include <OpenMesh/Core/IO/MeshIO.hh>

MyMesh mesh;

if (!OpenMesh::IO::read_mesh(mesh, "some input file")) 
{
  std::cerr << "read error\n";
  exit(1);
}

// do something with your mesh ...

if (!OpenMesh::IO::write_mesh(mesh, "some output file")) 
{
  std::cerr << "write error\n";
  exit(1);
}

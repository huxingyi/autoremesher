#include <iostream>
#include <algorithm>
#include <iterator>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Subdivider/Adaptive/CompositeT.hh>

// ----------------------------------------

using OpenMesh::Subdivider::Adaptive;

// ---------------------------------------- necessary types

OpenMesh::TriMesh_ArrayKernelT< CompositeTraits > MyMesh;
CompositeT< MyMesh >                              Subdivider;

// ---------------------------------------- setup a subdivider

int main(int argc, char **argv)
{
  MyMesh     mesh; // create mesh and fill it

  if (!OpenMesh::IO::read_mesh(mesh, argv[1]))
    return 1; // error reading mesh

  Subdivider subdivider(mesh); // bind subdivider to mesh

  // -------------------- add some rules

  // anonymous registration
  subdivider.add< Tvv3<MyMesh>::Handle >();
  subdivider.add< VF<MyMesh>::Handle   >();
  subdivider.add< FF<MyMesh>::Handle   >();

  // 'named' registration
  FVc<MyMesh>::Handle hFVc;
  subdivider.add( hFVc );

  // print pre-computed coefficients to std::cout...
  std::copy(subdivider.rule( hFVc ).coeffs().begin(),
            subdivider.rule( hFVc ).coeffs().end(),
            std::ostream_iterator<double>(std::cout, ", "));

  // prepare subdivider and the traits
  if (!subdivider.initialize())
    return 1; // error initializing subdivider

  MyMesh::FaceHandle fh; // select a face
  subdivider.refine(fh); 
}

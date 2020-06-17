
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/DefaultTriMesh.hh>
#include <OpenMesh/Core/Utils/PropertyManager.hh>

#include <iostream>
#include <vector>

using MyMesh = OpenMesh::TriMesh;

int main(int argc, char** argv)
{
  // Read command line options
  MyMesh mesh;
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " #iterations infile outfile" << std::endl;
    return 1;
  }
  const int iterations = argv[1];
  const std::string infile = argv[2];
  const std::string outfile = argv[3];
  
  // Read mesh file
  if (!OpenMesh::IO::read_mesh(mesh, infile)) {
    std::cerr << "Error: Cannot read mesh from " << infile << std::endl;
    return 1;
  }
  
  {
    // Add a vertex property storing the laplace vector
    auto laplace = OpenMesh::VProp<MyMesh::Point>(mesh);
    
    // Add a vertex property storing the laplace of the laplace
    auto bi_laplace = OpenMesh::VProp<MyMesh::Point>(mesh);
    
    // Get a propertymanager of the points property of the mesh to use as functor
    auto points = OpenMesh::getPointsProperty(mesh);
    
    // Smooth the mesh several times
    for (int i = 0; i < iterations; ++i) {
      // Iterate over all vertices to compute laplace vector
      for (const auto& vh : mesh.vertices())
        laplace(vh) = vh.vertices().avg(points) - points(vh);
      
      // Iterate over all vertices to compute the laplace vector of the laplace vectors
      for (const auto& vh : mesh.vertices())
        bi_laplace(vh) =  (vh.vertices().avg(laplace) - laplace(vh));
      
      // update points by substracting the bi-laplacian damped by a factor of 0.5
      for (const auto& vh : mesh.vertices())
        points(vh) += -0.5 * bi_laplace(vh);
    }
  } // The laplace and update properties are removed from the mesh at the end of this scope.
  
  
  // Write mesh file
  if (!OpenMesh::IO::read_mesh(mesh, outfile)) {
    std::cerr << "Error: Cannot write mesh to " << outfile << std::endl;
    return 1;
  }
}


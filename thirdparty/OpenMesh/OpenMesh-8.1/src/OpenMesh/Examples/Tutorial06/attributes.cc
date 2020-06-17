#include <iostream>
#include <typeinfo>
// --------------------
#include <OpenMesh/Core/IO/MeshIO.hh>

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Geometry/VectorT.hh>

#ifndef DOXY_IGNORE_THIS

// Define my personal traits
struct MyTraits : OpenMesh::DefaultTraits
{
  // Let Point and Normal be a vector of doubles
  typedef OpenMesh::Vec3d Point;
  typedef OpenMesh::Vec3d Normal;

  // Already defined in OpenMesh::DefaultTraits
  // HalfedgeAttributes( OpenMesh::Attributes::PrevHalfedge );
  
  // Uncomment next line to disable attribute PrevHalfedge
  // HalfedgeAttributes( OpenMesh::Attributes::None );
  //
  // or
  //
  // HalfedgeAttributes( 0 );
};

#endif

// Define my mesh with the new traits!
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;

// ------------------------------------------------------------------ main ----

int main(int argc, char **argv)
{
  MyMesh mesh;

  if (argc!=2)
  {
    std::cerr << "Usage: " << argv[0] << " <input>\n";
    return 1;
  }

  // Just make sure that point element type is double
  if ( typeid( OpenMesh::vector_traits<MyMesh::Point>::value_type ) 
       != typeid(double) )
  {
    std::cerr << "Ouch! ERROR! Data type is wrong!\n";
    return 1;
  }

  // Make sure that normal element type is double
  if ( typeid( OpenMesh::vector_traits<MyMesh::Normal>::value_type ) 
       != typeid(double) )
  {
    std::cerr << "Ouch! ERROR! Data type is wrong!\n";
    return 1;
  }

  // Add vertex normals as default property (ref. previous tutorial)
  mesh.request_vertex_normals();

  // Add face normals as default property
  mesh.request_face_normals();

  // load a mesh
  OpenMesh::IO::Options opt;
  if ( ! OpenMesh::IO::read_mesh(mesh,argv[1], opt))
  {
    std::cerr << "Error loading mesh from file " << argv[1] << std::endl;
    return 1;
  }

  // If the file did not provide vertex normals, then calculate them
  if ( !opt.check( OpenMesh::IO::Options::VertexNormal ) &&
       mesh.has_face_normals() && mesh.has_vertex_normals() )
  {
    // let the mesh update the normals
    mesh.update_normals();
  }

  // move all vertices one unit length along it's normal direction
  for (MyMesh::VertexIter v_it = mesh.vertices_begin();
       v_it != mesh.vertices_end(); ++v_it)
  {
    std::cout << "Vertex #" << v_it << ": " << mesh.point( v_it );
    mesh.set_point( v_it, mesh.point(v_it)+mesh.normal(v_it) );
    std::cout << " moved to " << mesh.point( v_it ) << std::endl;
  }

  return 0;
}

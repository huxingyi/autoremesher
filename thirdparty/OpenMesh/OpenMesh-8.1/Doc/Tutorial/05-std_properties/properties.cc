#include <iostream>
// --------------------
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>


typedef OpenMesh::TriMesh_ArrayKernelT<>  MyMesh;


int main(int argc, char **argv)
{
  MyMesh  mesh;

  if (argc!=2)
  {
    std::cerr << "Usage: " << argv[0] << " <input>\n";
    return 1;
  }

  // request vertex normals, so the mesh reader can use normal information
  // if available
  mesh.request_vertex_normals();

  // assure we have vertex normals
  if (!mesh.has_vertex_normals())
  {
    std::cerr << "ERROR: Standard vertex property 'Normals' not available!\n";
    return 1;
  }

  OpenMesh::IO::Options opt;
  if ( ! OpenMesh::IO::read_mesh(mesh,argv[1], opt))
  {
    std::cerr << "Error loading mesh from file " << argv[1] << std::endl;
    return 1;
  }

  // If the file did not provide vertex normals, then calculate them
  if ( !opt.check( OpenMesh::IO::Options::VertexNormal ) )
  {
    // we need face normals to update the vertex normals
    mesh.request_face_normals();

    // let the mesh update the normals
    mesh.update_normals();

    // dispose the face normals, as we don't need them anymore
    mesh.release_face_normals();
  }

  // move all vertices one unit length along it's normal direction
  for (MyMesh::VertexIter v_it = mesh.vertices_begin();
       v_it != mesh.vertices_end(); ++v_it)
  {
    std::cout << "Vertex #" << *v_it << ": " << mesh.point( *v_it );
    mesh.set_point( *v_it, mesh.point(*v_it)+mesh.normal(*v_it) );
    std::cout << " moved to " << mesh.point( *v_it ) << std::endl;
  }

  // don't need the normals anymore? Remove them!
  mesh.release_vertex_normals();

  // just check if it really works
  if (mesh.has_vertex_normals())
  {
    std::cerr << "Ouch! ERROR! Shouldn't have any vertex normals anymore!\n";
    return 1;
  }

  return 0;
}

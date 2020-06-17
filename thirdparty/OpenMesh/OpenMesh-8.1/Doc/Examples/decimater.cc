using namespace OpenMesh;

typedef TriMesh_ArrayKernelT<>               Mesh;
typedef Decimater::DecimaterT<Mesh>          Decimater;
typedef Decimater::ModQuadricT<Mesh>::Handle HModQuadric;

Mesh        mesh;             // a mesh object
Decimater   decimater(mesh);  // a decimater object, connected to a mesh
HModQuadric hModQuadric;      // use a quadric module

decimater.add(hModQuadric); // register module at the decimater
std::cout << decimater.module(hModQuadric).name() << std::endl; // module access

/*
 * since we need exactly one priority module (non-binary)
 * we have to call set_binary(false) for our priority module
 * in the case of HModQuadric, unset_max_err() calls set_binary(false) internally
 */
decimater.module(hModQuadric).unset_max_err();

decimater.initialize();
decimater.decimate();

// after decimation: remove decimated elements from the mesh
mesh.garbage_collection();

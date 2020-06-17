
#include <gtest/gtest.h>
#include <OpenMesh/Core/Utils/PropertyManager.hh>
#include <Unittests/unittests_common.hh>
#include <string>
#include <map>
#include "generate_cube.hh"
#include "fill_props.hh"

/*
 * ====================================================================
 * Definition of custom properties related classes
 * ====================================================================
 */

struct MyData
{
  int             ival;
  double          dval;
  bool            bval;
  OpenMesh::Vec4f vec4fval;

  MyData()
    : ival(0), dval(0.0), bval(false)
  { }

  MyData( const MyData& _cpy )
    : ival(_cpy.ival), dval(_cpy.dval), bval(_cpy.bval),
      vec4fval(_cpy.vec4fval)
  { }

  // ---------- assignment

  MyData& operator = (const MyData& _rhs)
  {
    ival = _rhs.ival;
    dval = _rhs.dval;
    bval = _rhs.bval;
    vec4fval = _rhs.vec4fval;
    return *this;
  }

  MyData& operator = (int    _rhs) { ival = _rhs; return *this; }
  MyData& operator = (double _rhs) { dval = _rhs; return *this; }
  MyData& operator = (bool   _rhs) { bval = _rhs; return *this; }
  MyData& operator = (const OpenMesh::Vec4f& _rhs)
  { vec4fval = _rhs; return *this; }

  // ---------- comparison

  bool operator == (const MyData& _rhs) const
  {
    return ival == _rhs.ival
      &&   dval == _rhs.dval
      &&   bval == _rhs.bval
      &&   vec4fval == _rhs.vec4fval;
  }
  bool operator != (const MyData& _rhs) const { return !(*this == _rhs); }
};

typedef std::map< std::string, unsigned int > MyMap;

namespace OpenMesh {
  namespace IO {
    // support persistence for struct MyData
    template <> struct binary<MyData>
    {
      typedef MyData value_type;
      static const bool is_streamable = true;

      // return binary size of the value
      static size_t size_of(void)
      {
        return sizeof(int)+sizeof(double)+sizeof(bool)+sizeof(OpenMesh::Vec4f);
      }

      static size_t size_of(const value_type&)
      {
        return size_of();
      }

      static size_t store(std::ostream& _os, const value_type& _v, bool _swap=false)
      {
        size_t bytes;
        bytes  = IO::store( _os, _v.ival, _swap );
        bytes += IO::store( _os, _v.dval, _swap );
        bytes += IO::store( _os, _v.bval, _swap );
        bytes += IO::store( _os, _v.vec4fval, _swap );
        return _os.good() ? bytes : 0;
      }

      static size_t restore( std::istream& _is, value_type& _v, bool _swap=false)
      {
        size_t bytes;
        bytes  = IO::restore( _is, _v.ival, _swap );
        bytes += IO::restore( _is, _v.dval, _swap );
        bytes += IO::restore( _is, _v.bval, _swap );
        bytes += IO::restore( _is, _v.vec4fval, _swap );
        return _is.good() ? bytes : 0;
      }
    };

    template <> struct binary< MyMap >
    {
      typedef MyMap value_type;
      static const bool is_streamable = true;

      // return generic binary size of self, if known
      static size_t size_of(void) { return UnknownSize; }

      // return binary size of the value
      static size_t size_of(const value_type& _v)
      {
        if (_v.empty())
          return sizeof(unsigned int);

        value_type::const_iterator it = _v.begin();
        unsigned int   N     = _v.size();
        size_t         bytes = IO::size_of(N);
        for(;it!=_v.end(); ++it)
        {
          bytes += IO::size_of( it->first );
          bytes += IO::size_of( it->second );
        }
        return bytes;
      }

      static
      size_t store(std::ostream& _os, const value_type& _v, bool _swap=false)
      {
        size_t   bytes = 0;
        unsigned int N = _v.size();
        value_type::const_iterator it = _v.begin();
        bytes += IO::store( _os, N, _swap );
        for (; it != _v.end() && _os.good(); ++it)
        {
          bytes += IO::store( _os, it->first, _swap );
          bytes += IO::store( _os, it->second, _swap );
        }
        return _os.good() ? bytes : 0;
      }

      static
      size_t restore( std::istream& _is, value_type& _v, bool _swap=false)
      {
        size_t   bytes = 0;
        unsigned int N = 0;
        _v.clear();
        bytes += IO::restore( _is, N, _swap );
        value_type::key_type key;
        value_type::mapped_type  val;
        for (size_t i=0; i<N && _is.good(); ++i)
        {
          bytes += IO::restore( _is, key, _swap );
          bytes += IO::restore( _is, val, _swap );
          _v[key] = val;
        }
        return _is.good() ? bytes : 0;
      }
    };

  }
}

namespace {

class OpenMeshTutorials: public OpenMeshBase {

    protected:

        // This function is called before each test is run
        virtual void SetUp() {

            // Do some initial stuff with the member data here...
        }

        // This function is called after all tests are through
        virtual void TearDown() {

            // Do some final stuff with the member data here...
        }

    // Member already defined in OpenMeshBase
    //Mesh mesh_;
};

/*
 * ====================================================================
 * Classes for unittests
 * ====================================================================
 */

template <class Mesh> class SmootherT
{
public:
  typedef typename Mesh::Point            cog_t;
  typedef OpenMesh::VPropHandleT< cog_t > Property_cog;

public:
  // construct with a given mesh
  explicit SmootherT(Mesh& _mesh)
    : mesh_(_mesh)
  {
    mesh_.add_property( cog_ );
  }
  ~SmootherT()
  {
    mesh_.remove_property( cog_ );
  }
  // smooth mesh _iterations times
  void smooth(unsigned int _iterations)
  {
    for (unsigned int i=0; i < _iterations; ++i)
    {
      std::for_each(mesh_.vertices_begin(),
                    mesh_.vertices_end(),
                    ComputeCOG(mesh_, cog_));
      std::for_each(mesh_.vertices_begin(),
                    mesh_.vertices_end(),
                    SetCOG(mesh_, cog_));
    }
  }

private:
  //--- private classes ---
  class ComputeCOG
  {
  public:
    ComputeCOG(Mesh& _mesh, Property_cog& _cog)
      : mesh_(_mesh), cog_(_cog)
    {}
    void operator()(const typename Mesh::VertexHandle& _vh)
    {
      typename Mesh::VertexVertexIter  vv_it;
      typename Mesh::Scalar            valence(0.0);

      mesh_.property(cog_, _vh) = typename Mesh::Point(0.0, 0.0, 0.0);
      for (vv_it=mesh_.vv_iter(_vh); vv_it.is_valid(); ++vv_it)
      {
        mesh_.property(cog_, _vh) += mesh_.point( *vv_it );
        ++valence;
      }
      mesh_.property(cog_, _vh ) /= valence;
    }
  private:
    Mesh&         mesh_;
    Property_cog& cog_;
  };

  class SetCOG
  {
  public:
    SetCOG(Mesh& _mesh, Property_cog& _cog)
      : mesh_(_mesh), cog_(_cog)
    {}
    void operator()(const typename Mesh::VertexHandle& _vh)
    {
      if (!mesh_.is_boundary(_vh))
        mesh_.set_point( _vh, mesh_.property(cog_, _vh) );
    }
  private:
    Mesh&         mesh_;
    Property_cog& cog_;
  };

  //--- private elements ---
  Mesh&        mesh_;
  Property_cog cog_;
};


/*
 * ====================================================================
 * Specify our traits
 * ====================================================================
 */

struct MyTraits : public OpenMesh::DefaultTraits
{
  HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
};

// Define my personal fancy traits
struct MyFancyTraits : OpenMesh::DefaultTraits
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

struct MyTraitsWithCOG : public OpenMesh::DefaultTraits
{
  // store barycenter of neighbors in this member
  VertexTraits
  {
  private:
    Point  cog_;
  public:
    VertexT() : cog_( Point(0.0f, 0.0f, 0.0f ) ) { }
    const Point& cog() const { return cog_; }
    void set_cog(const Point& _p) { cog_ = _p; }
  };
};

struct MyTraitsWithStatus : public OpenMesh::DefaultTraits
{
  VertexAttributes(OpenMesh::Attributes::Status);
  FaceAttributes(OpenMesh::Attributes::Status);
  EdgeAttributes(OpenMesh::Attributes::Status);
};

/*
 * ====================================================================
 * Specify our meshes
 * ====================================================================
 */
typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits> MyMeshWithTraits;
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits> MyTriMesh;
typedef OpenMesh::TriMesh_ArrayKernelT<MyFancyTraits>  MyFancyTriMesh;
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraitsWithCOG>  MyTriMeshWithCOG;
typedef OpenMesh::PolyMesh_ArrayKernelT<MyTraitsWithStatus> MyMeshWithStatus;

/*
 * ====================================================================
 * Define tests below
 * ====================================================================
 */

/*
 */
TEST_F(OpenMeshTutorials, building_a_cube) {

  MyMesh mesh;

  // generate vertices
  MyMesh::VertexHandle vhandle[8];
  vhandle[0] = mesh.add_vertex(MyMesh::Point(-1, -1,  1));
  vhandle[1] = mesh.add_vertex(MyMesh::Point( 1, -1,  1));
  vhandle[2] = mesh.add_vertex(MyMesh::Point( 1,  1,  1));
  vhandle[3] = mesh.add_vertex(MyMesh::Point(-1,  1,  1));
  vhandle[4] = mesh.add_vertex(MyMesh::Point(-1, -1, -1));
  vhandle[5] = mesh.add_vertex(MyMesh::Point( 1, -1, -1));
  vhandle[6] = mesh.add_vertex(MyMesh::Point( 1,  1, -1));
  vhandle[7] = mesh.add_vertex(MyMesh::Point(-1,  1, -1));

  // generate (quadrilateral) faces
  std::vector<MyMesh::VertexHandle>  face_vhandles;
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh.add_face(face_vhandles);

  face_vhandles.clear();
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[4]);
  mesh.add_face(face_vhandles);
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[4]);
  face_vhandles.push_back(vhandle[5]);
  mesh.add_face(face_vhandles);
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[6]);
  mesh.add_face(face_vhandles);
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[7]);
  mesh.add_face(face_vhandles);
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[7]);
  face_vhandles.push_back(vhandle[4]);
  mesh.add_face(face_vhandles);

  bool ok = OpenMesh::IO::write_mesh(mesh, "output.off");

  EXPECT_TRUE(ok) << "Cannot write mesh to file 'output.off'";
}

TEST_F(OpenMeshTutorials, using_iterators_and_circulators) {
  MyMesh  mesh;

  bool ok = OpenMesh::IO::read_mesh(mesh, "output.off");

  EXPECT_TRUE(ok) << "Cannot read mesh from file 'output.off'";

  // this vector stores the computed centers of gravity
  std::vector<MyMesh::Point>  cogs;
  std::vector<MyMesh::Point>::iterator cog_it;
  cogs.reserve(mesh.n_vertices());

  // smoothing mesh N times
  MyMesh::VertexIter          v_it, v_end(mesh.vertices_end());
  MyMesh::VertexVertexIter    vv_it;
  MyMesh::Point               cog;
  MyMesh::Scalar              valence;
  unsigned int                i, N(100);
  for (i=0; i < N; ++i)
  {
    cogs.clear();
    for (v_it = mesh.vertices_begin(); v_it != v_end; ++v_it)
    {
      cog[0] = cog[1] = cog[2] = valence = 0.0;

      for (vv_it = mesh.vv_iter( *v_it ); vv_it.is_valid(); ++vv_it)
      {
        cog += mesh.point( *vv_it );
        ++valence;
      }
      cogs.push_back(cog / valence);
    }

    for (v_it = mesh.vertices_begin(), cog_it = cogs.begin();
         v_it != v_end; ++v_it, ++cog_it)
      if ( !mesh.is_boundary( *v_it ) )
        mesh.set_point( *v_it, *cog_it );
  }

  // write mesh
  ok = OpenMesh::IO::write_mesh(mesh, "smoothed_output.off");

  EXPECT_TRUE(ok) << "Cannot write mesh to file 'smoothed_output.off'";
}

TEST_F(OpenMeshTutorials, using_custom_properties) {
  MyMesh  mesh;

  bool ok = OpenMesh::IO::read_mesh(mesh, "cube_noisy.off");
  EXPECT_TRUE(ok) << "Cannot read mesh from file 'cube_noisy.off'";

  const int iterations = 100;

  {
    // Add a vertex property storing the computed centers of gravity
    auto cog = OpenMesh::VProp<MyMesh::Point>(mesh);

    // Smooth the mesh several times
    for (int i = 0; i < iterations; ++i) {
      // Iterate over all vertices to compute centers of gravity
      for (const auto& vh : mesh.vertices()) {
        cog[vh] = {0,0,0};
        int valence = 0;
        // Iterate over all 1-ring vertices around vh
        for (const auto& vvh : mesh.vv_range(vh)) {
          cog[vh] += mesh.point(vvh);
          ++valence;
        }
        cog[vh] /= valence;
      }
      // Move all vertices to the previously computed positions
      for (const auto& vh : mesh.vertices()) {
        mesh.point(vh) = cog[vh];
      }
    }
  } // The cog vertex property is removed from the mesh at the end of this scope

  // write mesh
  ok = OpenMesh::IO::write_mesh(mesh, "smoothed_custom_properties_output.off");

  EXPECT_TRUE(ok) << "Cannot write mesh to file 'smoothed_custom_properties_output.off'";
}

TEST_F(OpenMeshTutorials, using_STL_algorithms) {
  MyMeshWithTraits mesh;

  bool ok = OpenMesh::IO::read_mesh(mesh, "cube_noisy.off");
  EXPECT_TRUE(ok) << "Cannot read mesh from file 'cube_noisy.off'";

  SmootherT<MyMeshWithTraits> smoother(mesh);
  smoother.smooth(100);

  // write mesh
  ok = OpenMesh::IO::write_mesh(mesh, "smoothed_STL_output.off");

  EXPECT_TRUE(ok) << "Cannot write mesh to file 'smoothed_STL_output.off'";
}

TEST_F(OpenMeshTutorials, using_standard_properties) {
  MyTriMesh mesh;

  mesh.request_vertex_normals();
  EXPECT_TRUE(mesh.has_vertex_normals()) << "Standard vertex property 'Normals' not available";

  OpenMesh::IO::Options opt;
  bool ok = OpenMesh::IO::read_mesh(mesh, "output.off", opt);
  EXPECT_TRUE(ok) << "Cannot read mesh from file 'output.off'";

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
    mesh.set_point( *v_it, mesh.point(*v_it)+mesh.normal(*v_it) );
  }

  // don't need the normals anymore? Remove them!
  mesh.release_vertex_normals();
  // just check if it really works
  EXPECT_FALSE(mesh.has_vertex_normals()) << "Shouldn't have any vertex normals anymore";
}

TEST_F(OpenMeshTutorials, using_mesh_attributes_and_traits) {
  MyFancyTriMesh mesh;

  // Just make sure that point element type is double
  EXPECT_TRUE(typeid( OpenMesh::vector_traits<MyFancyTriMesh::Point>::value_type ) ==
      typeid(double)) << "Data type is wrong";

  // Make sure that normal element type is double
  EXPECT_TRUE(typeid( OpenMesh::vector_traits<MyFancyTriMesh::Normal>::value_type ) ==
      typeid(double)) << "Data type is wrong";

  // Add vertex normals as default property (ref. previous tutorial)
  mesh.request_vertex_normals();
  // Add face normals as default property
  mesh.request_face_normals();

  // load a mesh
  OpenMesh::IO::Options opt;
  bool ok = OpenMesh::IO::read_mesh(mesh, "output.off", opt);
  EXPECT_TRUE(ok) << "Cannot read mesh from file 'output.off'";

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
    mesh.set_point( *v_it, mesh.point(*v_it)+mesh.normal(*v_it) );
  }
}

TEST_F(OpenMeshTutorials, extending_the_mesh_using_traits) {
  MyTriMeshWithCOG mesh;

  bool ok = OpenMesh::IO::read_mesh(mesh, "output.off");
  EXPECT_TRUE(ok) << "Cannot read mesh from file 'output.off'";

  // smoothing mesh N times
  MyTriMeshWithCOG::VertexIter          v_it, v_end(mesh.vertices_end());
  MyTriMeshWithCOG::VertexVertexIter    vv_it;
  MyTriMeshWithCOG::Point               cog;
  MyTriMeshWithCOG::Scalar              valence;
  unsigned int                i, N(100);

  for (i=0; i < N; ++i)
  {
    for (v_it = mesh.vertices_begin(); v_it != v_end; ++v_it)
    {
      cog[0] = cog[1] = cog[2] = valence = 0.0;

      for (vv_it = mesh.vv_iter(*v_it); vv_it.is_valid(); ++vv_it)
      {
        cog += mesh.point( *vv_it );
        ++valence;
      }
      mesh.data(*v_it).set_cog(cog / valence);
    }

    for (v_it = mesh.vertices_begin(); v_it != v_end; ++v_it)
      if (!mesh.is_boundary(*v_it))
        mesh.set_point( *v_it, mesh.data(*v_it).cog());
  }

  // write mesh
  ok = OpenMesh::IO::write_mesh(mesh, "smoothed_extended_output.off");

  EXPECT_TRUE(ok) << "Cannot write mesh to file 'smoothed_extended_output.off'";
}


TEST_F(OpenMeshTutorials, deleting_geometry_elements) {
  Mesh mesh;

  // the request has to be called before a vertex/face/edge can be deleted. it grants access to the status attribute
  mesh.request_face_status();
  mesh.request_edge_status();
  mesh.request_vertex_status();

  // generate vertices
  Mesh::VertexHandle vhandle[8];
  Mesh::FaceHandle   fhandle[6];

  vhandle[0] = mesh.add_vertex(Mesh::Point(-1, -1,  1));
  vhandle[1] = mesh.add_vertex(Mesh::Point( 1, -1,  1));
  vhandle[2] = mesh.add_vertex(Mesh::Point( 1,  1,  1));
  vhandle[3] = mesh.add_vertex(Mesh::Point(-1,  1,  1));
  vhandle[4] = mesh.add_vertex(Mesh::Point(-1, -1, -1));
  vhandle[5] = mesh.add_vertex(Mesh::Point( 1, -1, -1));
  vhandle[6] = mesh.add_vertex(Mesh::Point( 1,  1, -1));
  vhandle[7] = mesh.add_vertex(Mesh::Point(-1,  1, -1));

  // generate (quadrilateral) faces
  std::vector<Mesh::VertexHandle>  tmp_face_vhandles;
  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[0]);
  tmp_face_vhandles.push_back(vhandle[1]);
  tmp_face_vhandles.push_back(vhandle[2]);
  tmp_face_vhandles.push_back(vhandle[3]);
  fhandle[0] = mesh.add_face(tmp_face_vhandles);

  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[7]);
  tmp_face_vhandles.push_back(vhandle[6]);
  tmp_face_vhandles.push_back(vhandle[5]);
  tmp_face_vhandles.push_back(vhandle[4]);
  fhandle[1] = mesh.add_face(tmp_face_vhandles);

  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[1]);
  tmp_face_vhandles.push_back(vhandle[0]);
  tmp_face_vhandles.push_back(vhandle[4]);
  tmp_face_vhandles.push_back(vhandle[5]);
  fhandle[2] = mesh.add_face(tmp_face_vhandles);

  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[2]);
  tmp_face_vhandles.push_back(vhandle[1]);
  tmp_face_vhandles.push_back(vhandle[5]);
  tmp_face_vhandles.push_back(vhandle[6]);
  fhandle[3] = mesh.add_face(tmp_face_vhandles);
  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[3]);
  tmp_face_vhandles.push_back(vhandle[2]);
  tmp_face_vhandles.push_back(vhandle[6]);
  tmp_face_vhandles.push_back(vhandle[7]);
  fhandle[4] = mesh.add_face(tmp_face_vhandles);

  tmp_face_vhandles.clear();
  tmp_face_vhandles.push_back(vhandle[0]);
  tmp_face_vhandles.push_back(vhandle[3]);
  tmp_face_vhandles.push_back(vhandle[7]);
  tmp_face_vhandles.push_back(vhandle[4]);
  fhandle[5] = mesh.add_face(tmp_face_vhandles);

  // And now delete all faces and vertices
  // except face (vh[7], vh[6], vh[5], vh[4])
  // whose handle resides in fhandle[1]

  EXPECT_FALSE(mesh.status(fhandle[0]).deleted()) << "face shouldn't be deleted";
  EXPECT_FALSE(mesh.status(fhandle[1]).deleted()) << "face shouldn't be deleted";
  EXPECT_FALSE(mesh.status(fhandle[2]).deleted()) << "face shouldn't be deleted";
  EXPECT_FALSE(mesh.status(fhandle[3]).deleted()) << "face shouldn't be deleted";
  EXPECT_FALSE(mesh.status(fhandle[4]).deleted()) << "face shouldn't be deleted";
  EXPECT_FALSE(mesh.status(fhandle[5]).deleted()) << "face shouldn't be deleted";

  // Delete face 0
  mesh.delete_face(fhandle[0], false);
  // ... face 2
  mesh.delete_face(fhandle[2], false);
  // ... face 3
  mesh.delete_face(fhandle[3], false);
  // ... face 4
  mesh.delete_face(fhandle[4], false);
  // ... face 5
  mesh.delete_face(fhandle[5], false);

  EXPECT_TRUE(mesh.status(fhandle[0]).deleted()) << "face should be deleted";
  EXPECT_FALSE(mesh.status(fhandle[1]).deleted()) << "face shouldn't be deleted";
  EXPECT_TRUE(mesh.status(fhandle[2]).deleted()) << "face should be deleted";
  EXPECT_TRUE(mesh.status(fhandle[3]).deleted()) << "face should be deleted";
  EXPECT_TRUE(mesh.status(fhandle[4]).deleted()) << "face should be deleted";
  EXPECT_TRUE(mesh.status(fhandle[5]).deleted()) << "face should be deleted";

  // If isolated vertices result in a face deletion
  // they have to be deleted manually. If you want this
  // to happen automatically, change the second parameter
  // to true.
  // Now delete the isolated vertices 0, 1, 2 and 3

  EXPECT_FALSE(mesh.status(vhandle[0]).deleted()) << "vertex shouldn't be deleted";
  EXPECT_FALSE(mesh.status(vhandle[1]).deleted()) << "vertex shouldn't be deleted";
  EXPECT_FALSE(mesh.status(vhandle[2]).deleted()) << "vertex shouldn't be deleted";
  EXPECT_FALSE(mesh.status(vhandle[3]).deleted()) << "vertex shouldn't be deleted";


  mesh.delete_vertex(vhandle[0], false);
  mesh.delete_vertex(vhandle[1], false);
  mesh.delete_vertex(vhandle[2], false);
  mesh.delete_vertex(vhandle[3], false);


  EXPECT_TRUE(mesh.status(vhandle[0]).deleted()) << "vertex should be deleted";
  EXPECT_TRUE(mesh.status(vhandle[1]).deleted()) << "vertex should be deleted";
  EXPECT_TRUE(mesh.status(vhandle[2]).deleted()) << "vertex should be deleted";
  EXPECT_TRUE(mesh.status(vhandle[3]).deleted()) << "vertex should be deleted";

  // Delete all elements that are marked as deleted
  // from memory.
  mesh.garbage_collection();

  // write mesh
  bool ok = OpenMesh::IO::write_mesh(mesh, "deleted_output.off");

  EXPECT_TRUE(ok) << "Cannot write mesh to file 'deleted_output.off'";
}


TEST_F(OpenMeshTutorials, storing_custom_properties) {
  MyMesh mesh;

  // generate a geometry
  generate_cube<MyMesh>(mesh);

  // define some custom properties
  OpenMesh::VPropHandleT<float>       vprop_float;
  OpenMesh::EPropHandleT<bool>        eprop_bool;
  OpenMesh::FPropHandleT<std::string> fprop_string;
  OpenMesh::HPropHandleT<MyData>      hprop_mydata;
  OpenMesh::MPropHandleT<MyMap>       mprop_map;

  // registrate them at the mesh object
  mesh.add_property(vprop_float,  "vprop_float");
  mesh.add_property(eprop_bool,   "eprop_bool");
  mesh.add_property(fprop_string, "fprop_string");
  mesh.add_property(hprop_mydata, "hprop_mydata");
  mesh.add_property(mprop_map,    "mprop_map");

  //fill the props
  fill_props(mesh, vprop_float);
  fill_props(mesh, eprop_bool);
  fill_props(mesh, fprop_string);
  fill_props(mesh, hprop_mydata);
  fill_props(mesh, mprop_map);

  EXPECT_TRUE(fill_props(mesh, vprop_float, true)) << "property not filled correctly";
  EXPECT_TRUE(fill_props(mesh, eprop_bool, true)) << "property not filled correctly";
  EXPECT_TRUE(fill_props(mesh, fprop_string, true)) << "property not filled correctly";
  EXPECT_TRUE(fill_props(mesh, hprop_mydata, true)) << "property not filled correctly";
  EXPECT_TRUE(fill_props(mesh, mprop_map, true)) << "property not filled correctly";

  //Set persistent flag
  mesh.property(vprop_float).set_persistent(true);
  EXPECT_TRUE(mesh.property(vprop_float).persistent()) << "property should be persistent";
  mesh.property(eprop_bool).set_persistent(true);
  EXPECT_TRUE(mesh.property(eprop_bool).persistent()) << "property should be persistent";
  mesh.property(fprop_string).set_persistent(true);
  EXPECT_TRUE(mesh.property(fprop_string).persistent()) << "property should be persistent";
  mesh.property(hprop_mydata).set_persistent(true);
  EXPECT_TRUE(mesh.property(hprop_mydata).persistent()) << "property should be persistent";
  mesh.mproperty(mprop_map).set_persistent(true);
  EXPECT_TRUE(mesh.mproperty(mprop_map).persistent()) << "property should be persistent";

  // write mesh
  bool ok = OpenMesh::IO::write_mesh( mesh, "persistence-check.om" );
  EXPECT_TRUE(ok) << "Cannot write mesh to file 'persistent-check.om'";

  // clear mesh
  mesh.clear();

  //Read back mesh
  ok = OpenMesh::IO::read_mesh( mesh, "persistence-check.om" );
  EXPECT_TRUE(ok) << "Cannot read mesh from file 'persistent-check.om'";

  // check props
  EXPECT_TRUE(fill_props(mesh, vprop_float, true)) << "property not filled correctly";
  EXPECT_TRUE(fill_props(mesh, eprop_bool, true)) << "property not filled correctly";
  EXPECT_TRUE(fill_props(mesh, fprop_string, true)) << "property not filled correctly";
  EXPECT_TRUE(fill_props(mesh, hprop_mydata, true)) << "property not filled correctly";
  EXPECT_TRUE(fill_props(mesh, mprop_map, true)) << "property not filled correctly";
}

/*Testcase for code snippet from flipping edges in triangle meshes
 * */
TEST_F(OpenMeshTutorials, flipping_edges) {
  Mesh mesh;
  // Add some vertices
  Mesh::VertexHandle vhandle[4];
  vhandle[0] = mesh.add_vertex(Mesh::Point(0, 0, 0));
  vhandle[1] = mesh.add_vertex(Mesh::Point(0, 1, 0));
  vhandle[2] = mesh.add_vertex(Mesh::Point(1, 1, 0));
  vhandle[3] = mesh.add_vertex(Mesh::Point(1, 0, 0));
  // Add two faces
  std::vector<Mesh::VertexHandle> face_vhandles;
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  mesh.add_face(face_vhandles);
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[3]);
  mesh.add_face(face_vhandles);
  // Now the edge adjacent to the two faces connects
  // vertex vhandle[0] and vhandle[2].
  // Find this edge and then flip it
  for(Mesh::EdgeIter it = mesh.edges_begin(); it != mesh.edges_end(); ++it) {
    if(!mesh.is_boundary(*it)) {
      // Flip edge
      EXPECT_EQ(vhandle[2].idx(), mesh.to_vertex_handle(mesh.halfedge_handle(*it,0)).idx()) << "expected vertex handle 2!" ;
      EXPECT_EQ(vhandle[0].idx(), mesh.to_vertex_handle(mesh.halfedge_handle(*it,1)).idx()) << "expected vertex handle 0!" ;
      mesh.flip(*it);
      EXPECT_EQ(vhandle[1].idx(), mesh.to_vertex_handle(mesh.halfedge_handle(*it,0)).idx()) << "expected vertex handle 1 (did the flip work?)!" ;
      EXPECT_EQ(vhandle[3].idx(), mesh.to_vertex_handle(mesh.halfedge_handle(*it,1)).idx()) << "expected vertex handle 3 (did the flip work?)!" ;
    }
  }
  // The edge now connects vertex vhandle[1] and vhandle[3].
}

/*Testcase for code snippet from collapsing edges in triangle meshes
 * */
TEST_F(OpenMeshTutorials, collapsing_edges) {
  PolyMesh mesh;
  mesh.request_vertex_status();
  mesh.request_edge_status();
  // Add some vertices as in the illustration above
  PolyMesh::VertexHandle vhandle[7];
  vhandle[0] = mesh.add_vertex(PolyMesh::Point(-1, 1, 0));
  vhandle[1] = mesh.add_vertex(PolyMesh::Point(-1, 3, 0));
  vhandle[2] = mesh.add_vertex(PolyMesh::Point(0, 0, 0));
  vhandle[3] = mesh.add_vertex(PolyMesh::Point(0, 2, 0));
  vhandle[4] = mesh.add_vertex(PolyMesh::Point(0, 4, 0));
  vhandle[5] = mesh.add_vertex(PolyMesh::Point(1, 1, 0));
  vhandle[6] = mesh.add_vertex(PolyMesh::Point(1, 3, 0));
  // Add three quad faces
  std::vector<PolyMesh::VertexHandle> face_vhandles;
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[0]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[3]);
  mesh.add_face(face_vhandles);
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[1]);
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[5]);
  face_vhandles.push_back(vhandle[4]);
  mesh.add_face(face_vhandles);
  face_vhandles.clear();
  face_vhandles.push_back(vhandle[3]);
  face_vhandles.push_back(vhandle[2]);
  face_vhandles.push_back(vhandle[6]);
  face_vhandles.push_back(vhandle[5]);
  mesh.add_face(face_vhandles);
  // Now find the edge between vertex vhandle[2]
  // and vhandle[3]
  for(PolyMesh::HalfedgeIter it = mesh.halfedges_begin(); it != mesh.halfedges_end(); ++it) {
    if( mesh.to_vertex_handle(*it) == vhandle[3] &&
        mesh.from_vertex_handle(*it) == vhandle[2])
    {
      // Collapse edge
      mesh.collapse(*it);
      break;
    }
  }
  // Our mesh now looks like in the illustration above after the collapsing.
}

TEST_F(OpenMeshTutorials, using_smart_handles_and_smart_ranges) {
  MyMesh  mesh;

  bool ok = OpenMesh::IO::read_mesh(mesh, "cube_noisy.off");
  EXPECT_TRUE(ok) << "Cannot read mesh from file 'cube_noisy.off'";

  const int iterations = 100;

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

  // write mesh
  ok = OpenMesh::IO::write_mesh(mesh, "smoothed_smart_output.off");

  EXPECT_TRUE(ok) << "Cannot write mesh to file 'smoothed_smart_output.off'";
}

}

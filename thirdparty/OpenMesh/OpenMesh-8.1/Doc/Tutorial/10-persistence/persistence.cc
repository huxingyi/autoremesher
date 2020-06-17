#include <iostream>
#include <string>
#include <map>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
// -------------------- little helper
#include "generate_cube.hh"
#include "stats.hh"
#include "fill_props.hh"

// ----------------------------------------------------------------------------

// Set to 1 to use an PolyMesh type.
#define UsePolyMesh 1

// ----------------------------------------------------------------------------

using namespace OpenMesh;

// ----------------------------------------------------------------------------

typedef TriMesh_ArrayKernelT<>  TriMesh;
typedef PolyMesh_ArrayKernelT<> PolyMesh;

#if UsePolyMesh
typedef PolyMesh Mesh;
#else
typedef TriMesh Mesh;
#endif

// ----------------------------------------------------------------------------

#ifndef DOXY_IGNORE_THIS

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

#endif


// ----------------------------------------------------------------------------

typedef std::map< std::string, unsigned int > MyMap;


// ----------------------------------------------------------------------------

#ifndef DOXY_IGNORE_THIS

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

#endif


// ----------------------------------------------------------------------------

int main(void)
{
  //
  Mesh mesh;

  
  // generate a geometry
  generate_cube<Mesh>(mesh);


  // should display 8 vertices, 18/12 edges, 12/6 faces (Tri/Poly)
  mesh_stats(mesh);


  // print out information about properties
  mesh_property_stats(mesh);


  std::cout << "Define some custom properties..\n";

  OpenMesh::VPropHandleT<float>       vprop_float;
  OpenMesh::EPropHandleT<bool>        eprop_bool;
  OpenMesh::FPropHandleT<std::string> fprop_string;
  OpenMesh::HPropHandleT<MyData>      hprop_mydata;
  OpenMesh::MPropHandleT<MyMap>       mprop_map;
    
  std::cout << ".. and registrate them at the mesh object.\n";

  mesh.add_property(vprop_float,  "vprop_float");
  mesh.add_property(eprop_bool,   "eprop_bool");
  mesh.add_property(fprop_string, "fprop_string");
  mesh.add_property(hprop_mydata, "hprop_mydata");
  mesh.add_property(mprop_map,    "mprop_map");


  mesh_property_stats(mesh);


  std::cout << "Now let's fill the props..\n";

  fill_props(mesh, vprop_float);
  fill_props(mesh, eprop_bool);
  fill_props(mesh, fprop_string);
  fill_props(mesh, hprop_mydata);
  fill_props(mesh, mprop_map);


  std::cout << "Check props..\n";
#define CHK_PROP( PH ) \
  std::cout << "  " << #PH << " " \
            << (fill_props(mesh, PH, true)?"ok\n":"error\n")

  CHK_PROP(vprop_float);
  CHK_PROP(eprop_bool);
  CHK_PROP(fprop_string);
  CHK_PROP(hprop_mydata);
  CHK_PROP(mprop_map);
#undef CHK_PROP


  std::cout << "Set persistent flag..\n";
#define SET_PERS( PH ) \
    mesh.property(PH).set_persistent(true); \
    std::cout << "  " << #PH << " " \
              << (mesh.property(PH).persistent()?"ok\n":"failed!\n")

  mesh.property(vprop_float).set_persistent(true);
  std::cout << "  vprop_float "
            << (mesh.property(vprop_float).persistent()?"ok\n":"failed!\n");
  
  SET_PERS( eprop_bool );
  SET_PERS( fprop_string );
  SET_PERS( hprop_mydata );
  mesh.mproperty(mprop_map).set_persistent(true);
  std::cout << "  mprop_map "
            << (mesh.mproperty(mprop_map).persistent()?"ok\n":"failed!\n");
 

  std::cout << "Write mesh..";
  if (IO::write_mesh( mesh, "persistence-check.om" ))
    std::cout << "  ok\n";
  else
  {
    std::cout << "  failed\n";
    return 1;
  }


  std::cout << "Clear mesh\n";
  mesh.clear();
  mesh_stats(mesh, "  ");
  

  std::cout << "Read back mesh..";
  try
  {
    if (IO::read_mesh( mesh, "persistence-check.om" ))
      std::cout << "  ok\n";
    else
    {
      std::cout << "  failed!\n";
      return 1;
    }
    mesh_stats(mesh, "  ");
  }
  catch( std::exception &x )
  {
    std::cerr << x.what() << std::endl;
    return 1;
  }


  std::cout << "Check props..\n";
#define CHK_PROP( PH ) \
  std::cout << "  " << #PH << " " \
            << (fill_props(mesh, PH, true)?"ok\n":"error\n")
  CHK_PROP(vprop_float);
  CHK_PROP(eprop_bool);
  CHK_PROP(fprop_string);
  CHK_PROP(hprop_mydata);
  CHK_PROP(mprop_map);
#undef CHK_PROP

  return 0;
}

// end of file
// ============================================================================

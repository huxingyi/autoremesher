/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2015, RWTH-Aachen University                 *
 *           Department of Computer Graphics and Multimedia                  *
 *                          All rights reserved.                             *
 *                            www.openmesh.org                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * This file is part of OpenMesh.                                            *
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 *    this list of conditions and the following disclaimer.                  *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 *    contributors may be used to endorse or promote products derived from   *
 *    this software without specific prior written permission.               *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * ========================================================================= */



#include <iostream>
#include <iterator>
#include <fstream>
#include <string>
//
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Tools/Utils/getopt.h>


struct MyTraits : public OpenMesh::DefaultTraits
{
  VertexAttributes  ( OpenMesh::Attributes::Normal       |
		      OpenMesh::Attributes::Color        |
                      OpenMesh::Attributes::TexCoord2D   );
  HalfedgeAttributes( OpenMesh::Attributes::PrevHalfedge );
  FaceAttributes    ( OpenMesh::Attributes::Normal       |
		      OpenMesh::Attributes::Color        );
};

  
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits> MyMesh;

void usage_and_exit(int xcode)
{
   using std::cout;
   using std::endl;
   
   cout << "\nUsage: mconvert [option] <input> [<output>]\n\n";
   cout << "   Convert from one 3D geometry format to another.\n"
        << "   Or simply display some information about the object\n"
        << "   stored in <input>.\n"
        << endl;
   cout << "Options:\n"
        << endl;
   cout << "  -b\tUse binary mode if supported by target format.\n" << endl;
   cout << "    -l\tStore least significant bit first (LSB, little endian).\n" << endl;
   cout << "    -m\tStore most significant bit first (MSB, big endian).\n" << endl;
   cout << "    -s\tSwap byte order.\n" << endl;
   cout << "  -B\tUse binary mode if supported by source format.\n" << endl;
   cout << "    -S\tSwap byte order of input data.\n" << endl;
   cout << "  -c\tCopy vertex color if provided by input.\n" << endl;
   cout << "  -d\tCopy face color if provided by input.\n" << endl;
   cout << "  -C\tTranslate object in its center-of-gravity.\n" << endl;
   cout << "  -n\tCopy vertex normals if provided by input. Else compute normals.\n" << endl;
   cout << "  -N\tReverse normal directions.\n" << endl;
   cout << "  -t\tCopy vertex texture coordinates if provided by input file.\n" << endl;
   cout << "  -T \"x y z\"\tTranslate object by vector (x, y, z)'\"\n"
        << std::endl;
   cout << endl;
   
   exit(xcode);
}

// ----------------------------------------------------------------------------

template <typename T> struct Option : std::pair< T, bool >
{
  typedef std::pair< T, bool >              Base;
  
  Option()
  { Base::second = false; }

  bool is_valid() const { return Base::second; }
  bool is_empty() const { return !Base::second; }

  operator T& () { return Base::first; }
  operator const T& () const { return Base::first; }

  Option& operator = ( const T& _rhs )
  { Base::first = _rhs; Base::second=true; return *this; }

  bool operator == ( const T& _rhs ) const
  { return Base::first == _rhs; }

  bool operator != ( const T& _rhs ) const
  { return Base::first != _rhs; }
};

template <typename T>
std::ostream& operator << (std::ostream& _os, const Option<T>& _opt )
{
  if (_opt.second) _os << _opt.first; else _os << "<null>";
  return _os;
}

template <typename T>
std::istream& operator >> (std::istream& _is, Option<T>& _opt )
{
  _is >> _opt.first; _opt.second = true;
  return _is;
}

// ----------------------------------------------------------------------------

int main(int argc, char *argv[] )
{
  // ------------------------------------------------------------ command line

  int c;
  std::string ifname, ofname;
  bool rev_normals = false;
  bool obj_center  = false;
  OpenMesh::IO::Options opt, ropt;

  Option< MyMesh::Point > tvec;

  while ( (c=getopt(argc, argv, "bBcdCi:hlmnNo:sStT:"))!=-1 )
  {
    switch(c)
    {
      case 'b': opt  += OpenMesh::IO::Options::Binary; break;
      case 'B': ropt += OpenMesh::IO::Options::Binary; break;
      case 'l': opt  += OpenMesh::IO::Options::LSB; break;
      case 'm': opt  += OpenMesh::IO::Options::MSB; break;
      case 's': opt  += OpenMesh::IO::Options::Swap; break;
      case 'S': ropt += OpenMesh::IO::Options::Swap; break;
      case 'n': opt  += OpenMesh::IO::Options::VertexNormal; break;
      case 'N': rev_normals = true; break;
      case 'C': obj_center  = true; break;
      case 'c': opt  += OpenMesh::IO::Options::VertexColor; break;
      case 'd': opt  += OpenMesh::IO::Options::FaceColor; break;
      case 't': opt  += OpenMesh::IO::Options::VertexTexCoord; break;
      case 'T': 
      {
        std::cout << optarg << std::endl;
        std::stringstream str; str << optarg;
        str >> tvec;
        std::cout << tvec << std::endl;
        break;
      }
      case 'i': ifname = optarg; break;
      case 'o': ofname = optarg; break;
      case 'h':
        usage_and_exit(0);
        break;
      case '?':
      default:
        usage_and_exit(1);
    }
  }

  if (ifname.empty())
  { 
    if (optind < argc)
      ifname = argv[optind++];
    else
      usage_and_exit(1);
  }

  MyMesh mesh;
  OpenMesh::Utils::Timer  timer;

  // ------------------------------------------------------------ read

  std::cout << "reading.." << std::endl;
  {
    bool rc;
    timer.start();
    rc = OpenMesh::IO::read_mesh( mesh, ifname, ropt );
    timer.stop();
    if (rc)
      std::cout << "  read in " << timer.as_string() << std::endl;
    else
    {
      std::cout << "  read failed\n" << std::endl;
      return 1;
    }       
    timer.reset();
  }


  // ---------------------------------------- some information about input
  std::cout << (ropt.check(OpenMesh::IO::Options::Binary) 
                ? "  source is binary\n"
                : "  source is ascii\n");   

  std::cout << "  #V " << mesh.n_vertices() << std::endl;
  std::cout << "  #E " << mesh.n_edges() << std::endl;
  std::cout << "  #F " << mesh.n_faces() << std::endl;

  if (ropt.vertex_has_texcoord())
    std::cout << "  has texture coordinates" << std::endl;
    
  if (ropt.vertex_has_normal())
    std::cout << "  has vertex normals" << std::endl;

  if (ropt.vertex_has_color())
    std::cout << "  has vertex colors" << std::endl;
    
  if (ropt.face_has_normal())
    std::cout << "  has face normals" << std::endl;

  if (ropt.face_has_color())
    std::cout << "  has face colors" << std::endl;

  // 
  if (ofname.empty())
  {
    if ( optind < argc )
      ofname = argv[optind++];
    else
      return 0;
  }

  // ------------------------------------------------------------ features

  // ---------------------------------------- compute normal feature
  if ( opt.vertex_has_normal() && !ropt.vertex_has_normal())
  {
    std::cout << "compute normals" << std::endl;

    timer.start();
    mesh.update_face_normals();
    timer.stop();
    std::cout << "  " << mesh.n_faces()
              << " face normals in " << timer.as_string() << std::endl;
    timer.reset();
       
    timer.start();
    mesh.update_vertex_normals();
    timer.stop();
    std::cout << "  " << mesh.n_vertices()
              << " vertex normals in " << timer.as_string() << std::endl;
    timer.reset();       
  }


  // ---------------------------------------- reverse normal feature
  if ( rev_normals && ropt.vertex_has_normal() )
  {
    std::cout << "reverse normal directions" << std::endl;
    timer.start();
    MyMesh::VertexIter vit = mesh.vertices_begin();
    for (; vit != mesh.vertices_end(); ++vit)
      mesh.set_normal( *vit, -mesh.normal( *vit ) );
    timer.stop();
    std::cout << "  " << mesh.n_vertices()
              << " vertex normals in " << timer.as_string() << std::endl;
    timer.reset();       
       
  }


  // ---------------------------------------- centering feature
  if ( obj_center )
  {
    OpenMesh::Vec3f cog(0,0,0);
    size_t nv;
    std::cout << "center object" << std::endl;
    timer.start();    
    MyMesh::VertexIter vit = mesh.vertices_begin();
    for (; vit != mesh.vertices_end(); ++vit)
      cog += mesh.point( *vit );
    timer.stop();
    nv   = mesh.n_vertices();
    cog *= 1.0f/mesh.n_vertices();
    std::cout << "  cog = [" << cog << "]'" << std::endl;
    if (cog.sqrnorm() > 0.8) // actually one should consider the size of object
    {
      vit = mesh.vertices_begin();
      timer.cont();
      for (; vit != mesh.vertices_end(); ++vit)
        mesh.set_point( *vit , mesh.point( *vit )-cog );
      timer.stop();
      nv += mesh.n_vertices();
    }
    else
      std::cout << "    already centered!" << std::endl;
    std::cout << "  visited " << nv
              << " vertices in " << timer.as_string() << std::endl;
    timer.reset();       
  }


  // ---------------------------------------- translate feature
  if ( tvec.is_valid() )
  {
    std::cout << "Translate object by " << tvec << std::endl;

    timer.start();
    MyMesh::VertexIter vit = mesh.vertices_begin();
    for (; vit != mesh.vertices_end(); ++vit)
      mesh.set_point( *vit , mesh.point( *vit ) + tvec.first );
    timer.stop();
    std::cout << "  moved " << mesh.n_vertices()
              << " vertices in " << timer.as_string() << std::endl;
  }

  // ---------------------------------------- color vertices feature
  if (  opt.check( OpenMesh::IO::Options::VertexColor ) &&
        !ropt.check( OpenMesh::IO::Options::VertexColor ) )
  {
    std::cout << "Color vertices" << std::endl;

    double d  = 256.0/double(mesh.n_vertices());
    double d2 = d/2.0;
    double r  = 0.0, g = 0.0, b = 255.0;
    timer.start();
    MyMesh::VertexIter vit = mesh.vertices_begin();
    for (; vit != mesh.vertices_end(); ++vit)
    {
      mesh.set_color( *vit , MyMesh::Color( std::min((int)(r+0.5),255),
                                            std::min((int)(g+0.5),255),
                                            std::max((int)(b+0.5),0) ) );
      r += d;
      g += d2;
      b -= d;
    }
    timer.stop();
    std::cout << "  colored " << mesh.n_vertices()
              << " vertices in " << timer.as_string() << std::endl;
  }

  // ---------------------------------------- color faces feature
  if (  opt.check( OpenMesh::IO::Options::FaceColor ) &&
        !ropt.check( OpenMesh::IO::Options::FaceColor ) )
  {
    std::cout << "Color faces" << std::endl;

    double d  = 256.0/double(mesh.n_faces());
    double d2 = d/2.0;
    double r  = 0.0, g = 50.0, b = 255.0;
    timer.start();
    MyMesh::FaceIter it = mesh.faces_begin();
    for (; it != mesh.faces_end(); ++it)
    {
      mesh.set_color( *it , MyMesh::Color( std::min((int)(r+0.5),255),
                                           std::min((int)(g+0.5),255),
                                           std::max((int)(b+0.5),0) ) );
      r += d2;
//       g += d2;
      b -= d;
    }
    timer.stop();
    std::cout << "  colored " << mesh.n_faces()
              << " faces in " << timer.as_string() << std::endl;
  }

  // ------------------------------------------------------------ write
  
  std::cout << "writing.." << std::endl;    
  {
    bool rc;
    timer.start();
    rc = OpenMesh::IO::write_mesh( mesh, ofname, opt );          
    timer.stop();
       
    if (!rc)
    {
      std::cerr << "  error writing mesh!" << std::endl;
      return 1;
    }
       
    // -------------------------------------- write output and some info
    if ( opt.check(OpenMesh::IO::Options::Binary) )
    {          
      std::cout << "  "
                << OpenMesh::IO::binary_size(mesh, ofname, opt)
                << std::endl;
    }
    if ( opt.vertex_has_normal() )
      std::cout << "  with vertex normals" << std::endl;
    if ( opt.vertex_has_color() )
      std::cout << "  with vertex colors" << std::endl;
    if ( opt.vertex_has_texcoord() )
      std::cout << "  with vertex texcoord" << std::endl;
    if ( opt.face_has_normal() )
      std::cout << "  with face normals" << std::endl;
    if ( opt.face_has_color() )
      std::cout << "  with face colors" << std::endl;
    std::cout << "  wrote in " << timer.as_string() << std::endl;
    timer.reset();       
  }

  return 0;
}

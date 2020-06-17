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
#include <sstream>
// ---------------------------------------- OpenMesh Stuff
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Tools/Utils/getopt.h>
// ---------------------------------------- Subdivider
#include <OpenMesh/Tools/Subdivider/Uniform/Sqrt3T.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/LoopT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/CompositeSqrt3T.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/CompositeLoopT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/Sqrt3InterpolatingSubdividerLabsikGreinerT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/ModifiedButterFlyT.hh>
#include <OpenMesh/Tools/Subdivider/Uniform/CatmullClarkT.hh>

// ----------------------------------------------------------------------------

using namespace OpenMesh::Subdivider;

typedef OpenMesh::Subdivider::Uniform::CompositeTraits       CTraits;
typedef OpenMesh::TriMesh_ArrayKernelT<CTraits>              CMesh;

typedef OpenMesh::TriMesh_ArrayKernelT<>                     Mesh;
typedef Uniform::Sqrt3T< Mesh >                              Sqrt3;
typedef Uniform::LoopT< Mesh >                               Loop;
typedef Uniform::CompositeSqrt3T< CMesh >                    CompositeSqrt3;
typedef Uniform::CompositeLoopT< CMesh >                     CompositeLoop;
typedef Uniform::InterpolatingSqrt3LGT< Mesh >               InterpolatingSqrt3LG;
typedef Uniform::ModifiedButterflyT< Mesh >                  ModifiedButterfly;
typedef Uniform::CatmullClarkT< Mesh >                       CatmullClark;

using OpenMesh::Utils::Timer;

// ----------------------------------------------------------------------------

std::map< std::string, double > timings;

// ----------------------------------------------------------------------------

template < typename Subdivider >
bool subdivide( typename Subdivider::mesh_t& _m, size_t _n, 
                Timer::Format _fmt )
{
  bool       rc;
  Timer      t;
  Subdivider subdivide;

  std::cout << "Subdivide " << _n 
            << " times with '" << subdivide.name() << "'\n";

  subdivide.attach(_m);
  t.start();
  rc=subdivide( _n );
  t.stop();
  subdivide.detach();

  if (rc)
  {
    std::cout << "  Done [" << t.as_string(_fmt) << "]\n";
    timings[subdivide.name()] = t.seconds();
  }
  else
    std::cout << "  Failed!\n";
  return rc;
}

// ----------------------------------------------------------------------------

void usage_and_exit(int _xcode);

// ----------------------------------------------------------------------------

template < typename Subdivider >
int mainT( size_t _n, 
           const std::string& _ifname, 
           const std::string& _ofname, 
           const Timer::Format _fmt )
{
  // -------------------- read mesh
  std::cout << "Read mesh from file " << _ifname << std::endl;

  typename Subdivider::mesh_t mesh;

  if ( OpenMesh::IO::read_mesh( mesh, _ifname ) )
    std::cout << "  Ok\n";
  else
  {
    std::cout << "  Failed!\n";
    return 1;
  }

  std::cout << "  #V " << mesh.n_vertices() 
            << ", #F " << mesh.n_faces()
            << ", #E " << mesh.n_edges() << std::endl;

  // -------------------- subdividing
  try
  {
    if (!subdivide< Subdivider >( mesh, _n, _fmt ))
      return 1;
  }
  catch(std::bad_alloc& x)
  {
    std::cerr << "Out of memory: " << x.what() << std::endl;
    return 1;
  }
  catch(std::exception& x)
  {
    std::cerr << x.what() << std::endl;
    return 1;
  }
  catch(...)
  {
    std::cerr << "Unknown exception!\n";
    return 1;
  }

  // -------------------- write mesh

  std::cout << "  #V " << mesh.n_vertices() 
            << ", #F " << mesh.n_faces()
            << ", #E " << mesh.n_edges() << std::endl;

  if ( !_ofname.empty() )
  {
    std::cout << "Write resulting mesh to file " << _ofname << "..";
    if (OpenMesh::IO::write_mesh(mesh, _ofname, OpenMesh::IO::Options::Binary))
    {
      std::cout << "ok\n";
    }
    else
    {
      std::cerr << "Failed! Could not write file!\n";
      return 1;
    }
  }

  return 0;
}

// ----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  int    c;

  bool        compare_all = false;
  size_t      n;
  std::string ifname;
  std::string ofname;

  enum {
    TypeSqrt3,
    TypeLoop,
    TypeCompSqrt3,
    TypeCompLoop,
    TypeLabsikGreiner,
    TypeModButterfly,
    TypeCatmullClark
  } st = TypeSqrt3;

  Timer::Format fmt = Timer::Automatic;

  while ( (c=getopt(argc, argv, "csSlLbBhf:"))!=-1 )
  {
    switch(c)
    {
      case 'c': compare_all=true; break;
      case 's': st = TypeSqrt3; break;
      case 'S': st = TypeCompSqrt3; break;
      case 'l': st = TypeLoop; break;
      case 'L': st = TypeCompLoop; break;
      case 'b': st = TypeLabsikGreiner; break;
      case 'B': st = TypeModButterfly; break;
      case 'C': st = TypeCatmullClark; std::cerr << "Not yet supported, as it needs a poly mesh!"; break;
      case 'f': 
      {
        switch(*optarg)
        {          
          case 'm': fmt = Timer::MSeconds; break;
          case 'c': fmt = Timer::HSeconds; break;
          case 's': fmt = Timer::Seconds;  break;
          case 'a':
          default:  fmt = Timer::Automatic; break;
        }
        break;
      }
      case 'h': usage_and_exit(0); break;
      case '?':
      default:  usage_and_exit(1);
    }
  }
  
  if (argc-optind < 2)
    usage_and_exit(1);

  // # iterations
  {
    std::stringstream str; str << argv[optind]; str >> n;
  }

  // input file
  ifname = argv[++optind];

  // output file, if provided
  if ( ++optind < argc )
    ofname = argv[optind];


  // --------------------
  if ( compare_all )
  {
    int rc;
    rc  = mainT<Sqrt3>                  ( n, ifname, "", fmt );
    rc += mainT<Loop>                   ( n, ifname, "", fmt );
    rc += mainT<CompositeSqrt3>         ( n, ifname, "", fmt );
    rc += mainT<CompositeLoop>          ( n, ifname, "", fmt );
    rc += mainT<InterpolatingSqrt3LG>   ( n, ifname, "", fmt );
    rc += mainT<ModifiedButterfly>      ( n, ifname, "", fmt );
    rc += mainT<CatmullClark>           ( n, ifname, "", fmt );
    
    if (rc)
      return rc;

    std::cout << std::endl;

    std::map< std::string, double >::iterator it;
    
    std::cout << "Timings:\n";
    for(it = timings.begin();it!=timings.end();++it)
      std::cout << it->first << ": " << Timer::as_string(it->second)
                << std::endl;
    std::cout << std::endl;
    std::cout << "Ratio composite/native algorithm:\n";
    std::cout << "sqrt(3): " 
              << timings["Uniform Composite Sqrt3"]/timings["Uniform Sqrt3"]
              << std::endl
              << "loop   : "
              << timings["Uniform Composite Loop"]/timings["Uniform Loop"]
              << std::endl
              << "Interpolating sqrt(3)   : "
              << timings["Uniform Interpolating Sqrt3"]/timings["Uniform Sqrt3"]
              << std::endl;
    return 0;
  }
  else switch(st)
  {
    case TypeSqrt3: 
      return mainT<Sqrt3>( n, ifname, ofname, fmt );
    case TypeLoop:  
      return mainT<Loop>( n, ifname, ofname, fmt );
    case TypeCompSqrt3: 
      return mainT<CompositeSqrt3>( n, ifname, ofname, fmt );
    case TypeCompLoop:  
      return mainT<CompositeLoop> ( n, ifname, ofname, fmt );
    case TypeLabsikGreiner:
      return mainT<InterpolatingSqrt3LG> ( n, ifname, ofname, fmt );
    case TypeModButterfly:
      return mainT<ModifiedButterfly> ( n, ifname, ofname, fmt );
    case TypeCatmullClark:
      return mainT<CatmullClark> ( n, ifname, ofname, fmt );
  }
  return 1;
}

// ----------------------------------------------------------------------------

void usage_and_exit(int _xcode)
{
  std::cout << "Usage: subdivide [Subdivider Type] #Iterations Input [Output].\n";
  std::cout << std::endl;
  std::cout << "Subdivider Type\n"
            << std::endl
            << "  -l\tLoop\n"
            << "  -L\tComposite Loop\n"
            << "  -s\tSqrt3\n"
            << "  -S\tComposite Sqrt3\n"
            << "  -b\tInterpolating Sqrt3 Labsik-Greiner\n"
            << "  -B\tModified Butterfly\n"
           // << "  -C\tCatmullClark\n"
            << std::endl;
  exit(_xcode);
}

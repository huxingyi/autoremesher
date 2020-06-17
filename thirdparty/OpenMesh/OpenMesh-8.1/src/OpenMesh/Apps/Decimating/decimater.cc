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



#if !defined(OM_USE_OSG)
#  define OM_USE_OSG 0
#endif

// ----------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <map>
//--------------------
#include <OpenMesh/Core/IO/MeshIO.hh>
//--------------------
#if OM_USE_OSG
#  include <OpenMesh/Tools/Kernel_OSG/TriMesh_OSGArrayKernelT.hh>
#else
#  include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#endif
#include <OpenMesh/Core/Utils/vector_cast.hh>
//--------------------
#include <OpenMesh/Tools/Utils/getopt.h>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModAspectRatioT.hh>
#include <OpenMesh/Tools/Decimater/ModEdgeLengthT.hh>
#include <OpenMesh/Tools/Decimater/ModHausdorffT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalDeviationT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalFlippingT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <OpenMesh/Tools/Decimater/ModIndependentSetsT.hh>
#include <OpenMesh/Tools/Decimater/ModRoundnessT.hh>

//----------------------------------------------------------------- traits ----

#if OM_USE_OSG
typedef OpenMesh::Kernel_OSG::Traits MyTraits;
#else
typedef OpenMesh::DefaultTraits MyTraits;
#endif

//------------------------------------------------------------------- mesh ----

#if OM_USE_OSG
typedef OpenMesh::Kernel_OSG::TriMesh_OSGArrayKernelT<MyTraits> ArrayTriMesh;
#else
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits> ArrayTriMesh;
#endif


//-------------------------------------------------------------- decimator ----

typedef OpenMesh::Decimater::DecimaterT<ArrayTriMesh>   Decimater;


//---------------------------------------------------------------- globals ----

int gverbose = 0;
int gdebug   = 0;


//--------------------------------------------------------------- forwards ----

void usage_and_exit(int xcode);


//--------------------------------------------------- decimater arguments  ----

#include "CmdOption.hh"


struct DecOptions
{
  DecOptions()
  : n_collapses(0)
  { }

  CmdOption<bool>        decorate_name;
  CmdOption<float>       n_collapses;

  CmdOption<float>       AR;   // Aspect ratio
  CmdOption<float>       EL;   // Edge length
  CmdOption<float>       HD;   // Hausdorff distance
  CmdOption<bool>        IS;   // Independent Sets
  CmdOption<float>       ND;   // Normal deviation
  CmdOption<float>       NF;   // Normal flipping
  CmdOption<std::string> PM;   // Progressive Mesh
  CmdOption<float>       Q;    // Quadrics
  CmdOption<float>       R;    // Roundness

  template <typename T>
  bool init( CmdOption<T>& _o, const std::string& _val )
  {
    if ( _val.empty() )
      _o.enable();
    else
    {
      std::istringstream istr( _val );

      T v;

      if ( (istr >> v).fail() )
        return false;

      _o = v;
    }
    return true;
  }


  bool parse_argument( const std::string& arg )
  {
    std::string::size_type pos = arg.find(':');

    std::string name;
    std::string value;

    if (pos == std::string::npos)
      name = arg;
    else
    {
      name  = arg.substr(0, pos);
      value = arg.substr(pos+1, arg.size());
    }
    strip(name);
    strip(value);

    if (name == "AR") return init(AR, value);
    if (name == "EL") return init(EL, value);
    if (name == "HD") return init(HD, value);
    if (name == "IS") return init(IS, value);
    if (name == "ND") return init(ND, value);
    if (name == "NF") return init(NF, value);
    if (name == "PM") return init(PM, value);
    if (name == "Q")  return init(Q,  value);
    if (name == "R")  return init(R,  value);
    return false;
  }

  std::string& strip(std::string & line)
  {
    std::string::size_type pos = 0;

    pos = line.find_last_not_of(" \t");

    if ( pos!=0 && pos!=std::string::npos )
    {
      ++pos;
      line.erase( pos, line.length()-pos );
    }

    pos = line.find_first_not_of(" \t");
    if ( pos!=0 && pos!=std::string::npos )
    {
      line.erase(0,pos);
    }

    return line;
  }

};

//----------------------------------------------------- decimater wrapper  ----
//
template <typename Mesh, typename DecimaterType>
bool
decimate(const std::string &_ifname,
         const std::string &_ofname,
         DecOptions        &_opt)
{
   using namespace std;

   Mesh                   mesh;   
   OpenMesh::IO::Options  readopt;
   OpenMesh::Utils::Timer timer;

   // ---------------------------------------- read source mesh
   {
     if (gverbose)
       clog << "source mesh: ";
     bool rc;

     if (gverbose)
       clog << _ifname << endl;
     if ( !(rc = OpenMesh::IO::read_mesh(mesh, _ifname, readopt)) )
     {
       cerr << "  ERROR: read failed!" << endl;
       return rc;
     }
   }

   // ---------------------------------------- do some decimation
   {
     // ---- 0 - For module NormalFlipping one needs face normals

     if ( !readopt.check( OpenMesh::IO::Options::FaceNormal ) )
     {
       if ( !mesh.has_face_normals() )
         mesh.request_face_normals();

       if (gverbose)
         clog << "  updating face normals" << endl;
       mesh.update_face_normals();
     }

     // ---- 1 - create decimater instance
     DecimaterType decimater( mesh );

     // ---- 2 - register modules
     if (gverbose)
       clog << "  register modules" << endl;



     typename OpenMesh::Decimater::ModAspectRatioT<Mesh>::Handle modAR;

     if (_opt.AR.is_enabled())
     {
       decimater.add(modAR);
       if (_opt.AR.has_value())
         decimater.module( modAR ).set_aspect_ratio( _opt.AR ) ;
     }

     typename OpenMesh::Decimater::ModEdgeLengthT<Mesh>::Handle modEL;

     if (_opt.EL.is_enabled())
     {
       decimater.add(modEL);
       if (_opt.EL.has_value())
         decimater.module( modEL ).set_edge_length( _opt.EL ) ;
       decimater.module(modEL).set_binary(false);
     }

     typename OpenMesh::Decimater::ModHausdorffT <Mesh>::Handle modHD;

     if (_opt.HD.is_enabled())
     {
       decimater.add(modHD);
       if (_opt.HD.has_value())
         decimater.module( modHD ).set_tolerance( _opt.HD ) ;

     }

     typename OpenMesh::Decimater::ModIndependentSetsT<Mesh>::Handle modIS;

     if ( _opt.IS.is_enabled() )
       decimater.add(modIS);

     typename OpenMesh::Decimater::ModNormalDeviationT<Mesh>::Handle modND;

     if (_opt.ND.is_enabled())
     {
       decimater.add(modND);
       if (_opt.ND.has_value())
         decimater.module( modND ).set_normal_deviation( _opt.ND );
       decimater.module( modND ).set_binary(false);
     }

     typename OpenMesh::Decimater::ModNormalFlippingT<Mesh>::Handle modNF;

     if (_opt.NF.is_enabled())
     {
       decimater.add(modNF);
       if (_opt.NF.has_value())
         decimater.module( modNF ).set_max_normal_deviation( _opt.NF );
     }


     typename OpenMesh::Decimater::ModProgMeshT<Mesh>::Handle       modPM;

     if ( _opt.PM.is_enabled() )
       decimater.add(modPM);

     typename OpenMesh::Decimater::ModQuadricT<Mesh>::Handle        modQ;

     if (_opt.Q.is_enabled())
     {
       decimater.add(modQ);
       if (_opt.Q.has_value())
         decimater.module( modQ ).set_max_err( _opt.Q );
       decimater.module(modQ).set_binary(false);
     }

     typename OpenMesh::Decimater::ModRoundnessT<Mesh>::Handle      modR;

     if ( _opt.R.is_enabled() )
     {
       decimater.add( modR );
       if ( _opt.R.has_value() )
         decimater.module( modR ).set_min_angle( _opt.R,
             !modQ.is_valid() ||
             !decimater.module(modQ).is_binary());
     }

     // ---- 3 - initialize decimater

     if (gverbose)
       clog << "initializing mesh" << endl;

     {
       bool rc;
       timer.start();
       rc = decimater.initialize();
       timer.stop();
       if (!rc)
       {
         std::cerr << "  initializing failed!" << std::endl;
         std::cerr << "  maybe no priority module or more than one were defined!" << std::endl;
         return false;
       }
     }
     if (gverbose)
       std::clog << "  Elapsed time: " << timer.as_string() << std::endl;

     if (gverbose)
       decimater.info( clog );

     // ---- 4 - do it

     if (gverbose)
     {
       std::clog << "decimating" << std::endl;
       std::clog << "  # vertices: "       << mesh.n_vertices() << std::endl;
     }

     float nv_before = float(mesh.n_vertices());

     timer.start();
     size_t rc = 0;
     if (_opt.n_collapses < 0.0)
       rc = decimater.decimate_to( size_t(-_opt.n_collapses) );
     else if (_opt.n_collapses >= 1.0 || _opt.n_collapses == 0.0)
       rc = decimater.decimate( size_t(_opt.n_collapses) );
     else if (_opt.n_collapses > 0.0f)
       rc = decimater.decimate_to(size_t(mesh.n_vertices()*_opt.n_collapses));
     timer.stop();

     // ---- 5 - write progmesh file for progviewer (before garbage collection!)

     if ( _opt.PM.has_value() )
       decimater.module(modPM).write( _opt.PM );

     // ---- 6 - throw away all tagged edges

     mesh.garbage_collection();

     if (gverbose)
     {       
       std::clog << "  # executed collapses: " << rc << std::endl;
       std::clog << "  # vertices: " << mesh.n_vertices() << ", " 
           << ( 100.0*mesh.n_vertices()/nv_before ) << "%\n";
       std::clog << "  Elapsed time: " << timer.as_string() << std::endl;
       std::clog << "  collapses/s : " << rc/timer.seconds() << std::endl;
     }

   }

   // write resulting mesh
   if ( ! _ofname.empty() )
   {
     std::string ofname(_ofname);

     std::string::size_type pos = ofname.rfind('.');
     if (pos == std::string::npos)
     {
       ofname += ".off";
       pos = ofname.rfind('.');
     }

     if ( _opt.decorate_name.is_enabled() )
     {
       std::stringstream s; s << mesh.n_vertices();
       std::string       n; s >> n;
       ofname.insert(  pos, "-");
       ofname.insert(++pos, n  );
     }

     OpenMesh::IO::Options writeopt;

     //opt += OpenMesh::IO::Options::Binary;

     if ( !OpenMesh::IO::write_mesh(mesh, ofname, writeopt ) )
     {
       std::cerr << "  Cannot write decimated mesh to file '" 
           << ofname << "'\n";
       return false;
     }
     std::clog << "  Exported decimated mesh to file '" << ofname << "'\n";
   }

   return true;
}

//------------------------------------------------------------------ main -----

int main(int argc, char* argv[])   
{
  std::string  ifname, ofname;
   
  DecOptions opt;

  //
#if OM_USE_OSG
  osg::osgInit( argc, argv );
#endif

  //---------------------------------------- parse command line
  {
    int c;

    while ( (c=getopt( argc, argv, "dDhi:M:n:o:v")) != -1 )
    {
      switch (c)
      {
        case 'D': opt.decorate_name = true;   break;
        case 'd': gdebug            = true;   break;
        case 'h': usage_and_exit(0); break;
        case 'i': ifname            = optarg; break;
        case 'M': opt.parse_argument( optarg ); break;
        case 'n': opt.n_collapses   = float(atof(optarg)); break;
        case 'o': ofname            = optarg; break;
        case 'v': gverbose          = true;   break;
        case '?':
        default:
          std::cerr << "FATAL: cannot process command line option!"
          << std::endl;
          exit(-1);
      }                  
    }
  }

  //----------------------------------------

  if ( (-1.0f < opt.n_collapses) &&  (opt.n_collapses < 0.0f) )
  {
    std::cerr << "Error: Option -n: invalid value argument!" << std::endl;
    usage_and_exit(2);
  }

  //----------------------------------------

  if (gverbose)
  {
    std::clog << "    Input file: " << ifname << std::endl;
    std::clog << "   Output file: " << ofname << std::endl;
    std::clog << "    #collapses: " << opt.n_collapses << std::endl;
  }


  //----------------------------------------


   
  if (gverbose)
  {
    std::clog << "Begin decimation" << std::endl;
  }
   
  bool rc = decimate<ArrayTriMesh, Decimater>( ifname, ofname, opt );

  if (gverbose)
  {
    if (!rc)
      std::clog << "Decimation failed!" << std::endl;
    else
      std::clog << "Decimation done." << std::endl;
  }

  //----------------------------------------
  return 0;
}


//-----------------------------------------------------------------------------

void usage_and_exit(int xcode)
{
  std::string errmsg;

  switch(xcode)
  {
    case 1: errmsg = "Option not supported!"; break;
    case 2: errmsg = "Invalid output file format!"; break;
  }

  std::cerr << std::endl;
  if (xcode) {
    std::cerr << "Error " << xcode << ": " << errmsg << std::endl << std::endl;
  }
  std::cerr << "Usage: decimator [Options] -i input-file -o output-file\n"
            << "  Decimating a mesh using quadrics and normal flipping.\n" << std::endl;
  std::cerr << "Options\n"  << std::endl;
  std::cerr << " -M \"{Module-Name}[:Value]}\"\n"
            << "    Use named module with eventually given parameterization\n"
            << "    Several modules can also be used in order to introduce further constraints\n"
            << "    Note that -M has to be given before each new module \n"
            << "    An example with ModQuadric as a priority module\n"
            << "    and ModRoundness as a binary module could look like this:\n"
            << "    commandlineDecimater -M Q -M R:40.0 -n 0.1 -i inputfile.obj -o outputfile.obj\n" << std::endl;
  std::cerr << " -n <N>\n"
            << "    N >= 1: do N halfedge collapses.\n"
            << "    N <=-1: decimate down to |N| vertices.\n"
            << " 0 < N < 1: decimate down to N%.\n" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Modules:\n\n";
  std::cerr << "  AR[:ratio]      - ModAspectRatio\n";
  std::cerr << "  EL[:legth]      - ModEdgeLength*\n";
  std::cerr << "  HD[:distance]   - ModHausdorff\n";
  std::cerr << "  IS              - ModIndependentSets\n";
  std::cerr << "  ND[:angle]      - ModNormalDeviation*\n";
  std::cerr << "  NF[:angle]      - ModNormalFlipping\n";
  std::cerr << "  PM[:file name]  - ModProgMesh\n";
  std::cerr << "  Q[:error]       - ModQuadric*\n";
  std::cerr << "  R[:angle]       - ModRoundness\n";
  std::cerr << "    0 < angle < 60\n";
  std::cerr << "  *: priority module. Decimater needs one of them (not more).\n";

  exit( xcode );
}



//                             end of file
//=============================================================================

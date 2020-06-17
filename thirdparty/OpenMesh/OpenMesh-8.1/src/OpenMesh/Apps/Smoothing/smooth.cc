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
#include <OpenMesh/Core/IO/MeshIO.hh> // include before kernel type!
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Tools/Smoother/JacobiLaplaceSmootherT.hh>
#include <OpenMesh/Tools/Utils/getopt.h>


using namespace OpenMesh;
using namespace Smoother;


struct MyTraits : public OpenMesh::DefaultTraits
{
#if 1
  typedef OpenMesh::Vec3f Point;
  typedef OpenMesh::Vec3f Normal;
#else
  typedef OpenMesh::Vec3d Point;
  typedef OpenMesh::Vec3d Normal;
#endif
};

typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;


//-----------------------------------------------------------------------------

void usage_and_exit(int _xcode)
{
  std::cout << std::endl;
  std::cout << "Usage: smooth [Options] <iterations> <input mesh> <output mesh>\n";
  std::cout << std::endl;
  std::cout << "Options \n"
            << std::endl
	    << "  -c <0|1> \t continuity (C0,C1). Default: C1\n"
            << "  -t \t\t smooth tangential direction. Default: Enabled\n"
            << "  -n \t\t smooth normal direction. Default: Enabled\n"
	    << std::endl;
  exit(_xcode);
}


//-----------------------------------------------------------------------------


int main(int argc, char **argv)
{
  int    c;

  MyMesh  mesh;
  OpenMesh::Utils::Timer t;
  std::string ifname;
  std::string ofname;

  SmootherT<MyMesh>::Continuity 
    continuity = SmootherT<MyMesh>::C1;

  SmootherT<MyMesh>::Component  
    component  = SmootherT<MyMesh>::Tangential_and_Normal;

  int iterations;

  // ---------------------------------------- evaluate command line

  while ( (c=getopt(argc, argv, "tnc:h"))!=-1 )
  {
    switch(c)
    {
      case 'c': {
	switch(*optarg)
        { 
	  case '0' : continuity = SmootherT<MyMesh>::C0; break;
	  case '1' : continuity = SmootherT<MyMesh>::C1; break;
	}
	break;
      }
      case 't':
	component = component==SmootherT<MyMesh>::Normal
	  ? SmootherT<MyMesh>::Tangential_and_Normal
	  : SmootherT<MyMesh>::Tangential;
	break;

      case 'n': 
	component = component==SmootherT<MyMesh>::Tangential 
	  ? SmootherT<MyMesh>::Tangential_and_Normal
	  : SmootherT<MyMesh>::Normal;
	break;

      case 'h': usage_and_exit(0); break;
      case '?':
      default:  usage_and_exit(1);
    }
  }
  
  if (argc-optind < 3)
    usage_and_exit(1);


  // # iterations
  {
    std::stringstream str; str << argv[optind]; str >> iterations;
  }


  // input file
  ifname = argv[++optind];


  // output file
  ofname = argv[++optind];


  OpenMesh::IO::Options opt;

  // ---------------------------------------- read mesh
  
  omout() << "read mesh..." << std::flush;
  t.start();
  OpenMesh::IO::read_mesh(mesh, ifname, opt);
  t.stop();
  omout() << "done (" << t.as_string() << ")\n";

  omout() << "  #V " << mesh.n_vertices() << std::endl;

  // ---------------------------------------- smooth

  JacobiLaplaceSmootherT<MyMesh> smoother(mesh);
  smoother.initialize(component,continuity);
 
  omout() << "smoothing..." << std::flush;

  t.start(); 
  smoother.smooth(iterations); 
  t.stop();

  omout() << "done (";
  omout() << t.seconds() << "s ~ ";
  omout() << t.as_string() << ", " 
        << (iterations*mesh.n_vertices())/t.seconds() << " Vertices/s)\n";

  // ---------------------------------------- write mesh

  omout() << "write mesh..." << std::flush;
  t.start();
  OpenMesh::IO::write_mesh(mesh, ofname, opt);
  t.stop();
  omout() << "done (" << t.as_string() << ")\n";

  return 0;
}

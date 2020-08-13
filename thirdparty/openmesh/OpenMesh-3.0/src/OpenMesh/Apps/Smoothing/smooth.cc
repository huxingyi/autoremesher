/*===========================================================================*\
 *                                                                           *
 *                               OpenMesh                                    *
 *      Copyright (C) 2001-2014 by Computer Graphics Group, RWTH Aachen      *
 *                           www.openmesh.org                                *
 *                                                                           *
 *---------------------------------------------------------------------------* 
 *  This file is part of OpenMesh.                                           *
 *                                                                           *
 *  OpenMesh is free software: you can redistribute it and/or modify         * 
 *  it under the terms of the GNU Lesser General Public License as           *
 *  published by the Free Software Foundation, either version 3 of           *
 *  the License, or (at your option) any later version with the              *
 *  following exceptions:                                                    *
 *                                                                           *
 *  If other files instantiate templates or use macros                       *
 *  or inline functions from this file, or you compile this file and         *
 *  link it with other files to produce an executable, this file does        *
 *  not by itself cause the resulting executable to be covered by the        *
 *  GNU Lesser General Public License. This exception does not however       *
 *  invalidate any other reasons why the executable file might be            *
 *  covered by the GNU Lesser General Public License.                        *
 *                                                                           *
 *  OpenMesh is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU LesserGeneral Public          *
 *  License along with OpenMesh.  If not,                                    *
 *  see <http://www.gnu.org/licenses/>.                                      *
 *                                                                           *
\*===========================================================================*/ 

/*===========================================================================*\
 *                                                                           *             
 *   $Revision$                                                         *
 *   $Date$                   *
 *                                                                           *
\*===========================================================================*/


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

      case 'h': usage_and_exit(0);
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

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

// -------------------------------------------------------------- includes ----

// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Tools/Utils/getopt.h>
// -------------------- OpenMesh Adaptive Composite Subdivider
#include <OpenMesh/Tools/Subdivider/Adaptive/Composite/CompositeT.hh>
#include <OpenMesh/Tools/Subdivider/Adaptive/Composite/RulesT.hh>
// -------------------- STL
#include <iostream>
#include <fstream>
#include <sstream>
#if defined(OM_CC_MIPS)
#  include <math.h>
#else
#  include <cmath>
   using std::pow;
#endif

  
using OpenMesh::Subdivider::Adaptive::CompositeTraits;

// define mesh, rule interface, and subdivider types
typedef OpenMesh::TriMesh_ArrayKernelT<CompositeTraits>         MyMesh;
typedef OpenMesh::Subdivider::Adaptive::RuleInterfaceT<MyMesh>  Rule;
typedef OpenMesh::Subdivider::Adaptive::CompositeT<MyMesh>      Subdivider;

// ----------------------------------------------------------------------------

using namespace OpenMesh::Subdivider;

// factory function to add a RULE to a subdivider
#define ADD_FN( RULE ) \
  bool add_ ## RULE( Subdivider& _sub ) \
  { return _sub.add< Adaptive:: RULE < MyMesh > >(); }

ADD_FN( Tvv3 );
ADD_FN( Tvv4 );
ADD_FN( VF   );
ADD_FN( FF   );
ADD_FN( FFc  );
ADD_FN( FV   );
ADD_FN( FVc  );
ADD_FN( VV   );
ADD_FN( VVc  );
ADD_FN( VE   );
ADD_FN( VdE  );
ADD_FN( VdEc );
ADD_FN( EV   );
ADD_FN( EVc  );
ADD_FN( EF   );
ADD_FN( FE   );
ADD_FN( EdE  );
ADD_FN( EdEc );

#undef ADD_FN

typedef bool (*add_rule_ft)( Subdivider& );

// map rule name to factory function
struct RuleMap : std::map< std::string, add_rule_ft >
{
  RuleMap()
  {
#define ADD( RULE ) \
    (*this)[ #RULE ] = add_##RULE;

    ADD( Tvv3 );
    ADD( Tvv4 );
    ADD( VF   );
    ADD( FF   );
    ADD( FFc  );
    ADD( FV   );
    ADD( FVc  );
    ADD( VV   );
    ADD( VVc  );
    ADD( VE   );
    ADD( VdE  );
    ADD( VdEc );
    ADD( EV   );
    ADD( EVc  );
    ADD( EF   );
    ADD( FE   );
    ADD( EdE  );
    ADD( EdEc );

#undef ADD
  }

} available_rules;


// ----------------------------------------------------------------------------

std::string basename( const std::string& _fname );
void usage_and_exit(const std::string& _fname, int xcode);

// ----------------------------------------------------------------------------


int main(int argc, char **argv)
{
  size_t      n_iter = 0;          // n iteration
  size_t      max_nv = size_t(-1); // max. number of vertices in the end
  std::string ifname;              // input mesh
  std::string ofname;              // output mesh
  std::string rule_sequence = "Tvv3 VF FF FVc"; // sqrt3 default
  bool        uniform = false;
  int         c;

  // ---------------------------------------- evaluate command line 
  while ( (c=getopt(argc, argv, "hlm:n:r:sU"))!=-1 )
  {
    switch(c)
    {
      case 's': rule_sequence = "Tvv3 VF FF FVc";       break; // sqrt3
      case 'l': rule_sequence = "Tvv4 VdE EVc VdE EVc"; break; // loop
      case 'n': { std::stringstream s; s << optarg; s >> n_iter; } break;
      case 'm': { std::stringstream s; s << optarg; s >> max_nv; } break;
      case 'r': rule_sequence = optarg; break;
      case 'U': uniform = true; break;
      case 'h': usage_and_exit(argv[0],0);
      case '?':
      default:  usage_and_exit(argv[0],1);
    }
  }

  if ( optind == argc )
    usage_and_exit(argv[0],2);

  if ( optind < argc )
    ifname = argv[optind++];

  if ( optind < argc )
    ofname = argv[optind++];

  // if ( optind < argc ) // too many arguments

  // ---------------------------------------- mesh and subdivider
  MyMesh                          mesh;
  Subdivider                      subdivider(mesh);


  // -------------------- read mesh from file
  std::cout << "Input mesh         : " << ifname << std::endl;
  if (!OpenMesh::IO::read_mesh(mesh, ifname))
  {
    std::cerr << "  Error reading file!\n";
    return 1;
  }

  // store orignal size of mesh
  size_t n_vertices = mesh.n_vertices();
  size_t n_edges    = mesh.n_edges();
  size_t n_faces    = mesh.n_faces();
  
  if ( n_iter > 0 )
    std::cout << "Desired #iterations: " << n_iter << std::endl;

  if ( max_nv < size_t(-1) )
  {
    std::cout << "Desired max. #V    : " << max_nv << std::endl;
    if (!n_iter )
      n_iter = size_t(-1);
  }


  // -------------------- Setup rule sequence
  {    
    std::stringstream s;
    std::string       token;    

    RuleMap::iterator it = available_rules.end();

    for (s << rule_sequence; s >> token; )
    {
      if ( (it=available_rules.find( token )) != available_rules.end() )
      {
        it->second( subdivider );
      }      
      else if ( token[0]=='(' && (subdivider.n_rules() > 0) )
      {
        std::string::size_type beg(1);
        if (token.length()==1)
        {
          s >> token;
          beg = 0;
        }

        std::string::size_type 
          end  = token.find_last_of(')');
        std::string::size_type 
          size = end==std::string::npos ? token.size()-beg : end-beg;
          
        std::stringstream v;
        MyMesh::Scalar coeff;
        std::cout << "  " << token << std::endl;
        std::cout << "  " << beg << " " << end << " " << size << std::endl;
        v << token.substr(beg, size);
        v >> coeff;
        std::cout << "  coeffecient " << coeff << std::endl;
        subdivider.rule( subdivider.n_rules()-1 ).set_coeff(coeff);

        if (end == std::string::npos)
        {
          s >> token;
          if (token[0]!=')')
          {
            std::cerr << "Syntax error: Missing ')'\n";
            return 1;
          }
        }
      }
      else
      {
        std::cerr << "Syntax error: " << token << "?\n";
        return 1;
      }
    }
  }

  std::cout << "Rule sequence      : " 
            << subdivider.rules_as_string() << std::endl;

  // -------------------- Initialize subdivider
  std::cout << "Initialize subdivider\n";
  if (!subdivider.initialize())
  {
    std::cerr << "  Error!\n";
    return 1;
  }

  // 
  MyMesh::FaceFaceIter   ff_it;
  double                 quality(0.0);

  // ---------------------------------------- subdivide
  std::cout << "\nSubdividing...\n";

  OpenMesh::Utils::Timer timer, timer2;
  size_t                 i;

  if ( uniform )
  { // unifom
    MyMesh::VertexHandle vh;
    MyMesh::VertexIter   v_it;
    MyMesh::FaceHandle   fh;
    MyMesh::FaceIter     f_it;


    // raise all vertices to target state
    timer.start();

    size_t n       = n_iter;
    size_t n_rules = subdivider.n_rules();

    i = 0;

    // calculate target states for faces and vertices
    size_t target1 = (n - 1) * n_rules + subdivider.subdiv_rule().number() + 1;
    size_t target2 = n * n_rules;

    for (f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it) {
      
      if (mesh.data(*f_it).state() < int(target1) ) {
        ++i;        
        fh = *f_it;
        timer2.start();
        subdivider.refine(fh);
        timer2.stop();
      }
    }

    for (v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it) {
      
      if (mesh.data(*v_it).state() < int(target2) ) {
        vh = *v_it;
        timer2.cont();
        subdivider.refine(vh);
        timer2.stop();
      }
    }
    timer.stop();    
  }
  else
  { // adaptive

    MyMesh::FaceIter   f_it;
    MyMesh::FaceHandle fh;

    std::vector<double> __acos;
    size_t              buckets(3000);
    double              range(2.0);
    double              range2bucket(buckets/range);

    for (i = 0; i < buckets; ++i)
      __acos.push_back( acos(-1.0 + i * range / buckets) );

    timer.start(); // total time needed

    //  n iterations or until desired number of vertices reached approx.
    for (i = 0; i < n_iter && mesh.n_vertices() < max_nv; ++i) 
    {
      mesh.update_face_normals();

      // calculate quality
      quality = 0.0;

      fh = *(mesh.faces_begin());

      // check every face
      for (f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it) {

        double face_quality = 0.0;
        int valence      = 0;

        for (ff_it = mesh.ff_iter(*f_it); ff_it.is_valid(); ++ff_it) {

          double temp_quality = OpenMesh::dot( mesh.normal(*f_it), mesh.normal(*ff_it) );

          if (temp_quality >= 1.0)
            temp_quality = .99;
          else if (temp_quality <= -1.0)
            temp_quality = -.99;
          temp_quality  = (1.0+temp_quality) * range2bucket;
          face_quality += __acos[int(temp_quality+.5)];

          ++valence;
        }

        face_quality /= valence;

        // calaculate face area
        MyMesh::Point  p1, p2, p3;
        MyMesh::Scalar area;

#define heh halfedge_handle
#define nheh next_halfedge_handle
#define tvh to_vertex_handle
#define fvh from_vertex_handle
        p1 = mesh.point(mesh.tvh(mesh.heh(*f_it)));
        p2 = mesh.point(mesh.fvh(mesh.heh(*f_it)));
        p3 = mesh.point(mesh.tvh(mesh.nheh(mesh.heh(*f_it))));
#undef heh
#undef nheh
#undef tvh
#undef fvh

        area = ((p2 - p1) % (p3 - p1)).norm();

        // weight face_quality
        face_quality *= pow(double(area), double(.1));
        //face_quality *= area;

        if (face_quality >= quality && !mesh.is_boundary(*f_it))
        {
          quality = face_quality;
          fh      = *f_it;
        }
      }

      // Subdivide Face
      timer2.cont();
      subdivider.refine(fh);
      timer2.stop();
    }

    // calculate time
    timer.stop();

  } // uniform/adaptive?

  // calculate maximum refinement level
  Adaptive::state_t max_level(0);
  
  for (MyMesh::VertexIter v_it = mesh.vertices_begin(); 
       v_it != mesh.vertices_end(); ++v_it) 
  {    
    if (mesh.data(*v_it).state() > max_level)
      max_level = mesh.data(*v_it).state();
  }


  // output results
  std::cout << "\nDid " << i << (uniform ? " uniform " : "" )
            << " subdivision steps in "
            << timer.as_string()
            << ", " << i/timer.seconds() << " steps/s\n";
  std::cout << "  only refinement: " << timer2.as_string()
            << ", " << i/timer2.seconds() << " steps/s\n\n";

  std::cout << "Before: ";
  std::cout << n_vertices << " Vertices, ";
  std::cout << n_edges << " Edges, ";
  std::cout << n_faces << " Faces. \n";

  std::cout << "Now   : ";
  std::cout << mesh.n_vertices() << " Vertices, ";
  std::cout << mesh.n_edges() << " Edges, ";
  std::cout << mesh.n_faces() << " Faces. \n\n";

  std::cout << "Maximum quality          : " << quality << std::endl;
  std::cout << "Maximum Subdivision Level: " << max_level/subdivider.n_rules()
            << std::endl << std::endl;

  // ---------------------------------------- write mesh to file
  {
    if ( ofname.empty() )
    {
      std::stringstream s;

      s << "result." << subdivider.rules_as_string("_")
        << "-" << i << "x.off";
      s >> ofname;
    }

    std::cout << "Output file: '" << ofname << "'.\n";
    if (!OpenMesh::IO::write_mesh(mesh, ofname, OpenMesh::IO::Options::Binary))
    {
      std::cerr << "  Error writing file!\n";
      return 1;
    }
  }
  return 0;
}

// ----------------------------------------------------------------------------
// helper

void usage_and_exit(const std::string& _fname, int xcode)
{
  using namespace std;

  cout << endl
       << "Usage: " << basename(_fname)
       << " [Options] input-mesh [output-mesh]\n\n";
  cout << "\tAdaptively refine an input-mesh. The refined mesh is stored in\n"
       << "\ta file named \"result.XXX.off\" (binary .off), if not specified\n"
       << "\texplicitely (optional 2nd parameter of command line).\n\n";
  cout << "Options:\n\n";
  cout << "-m <int>\n\tAdaptively refine up to approx. <int> vertices.\n\n"
       << "-n <int>\n\tAdaptively refine <int> times.\n\n"
       << "-r <rule sequence>\n\tDefine a custom rule sequence.\n\n"
       << "-l\n\tUse rule sequence for adaptive Loop.\n\n"
       << "-s\n\tUse rule sequence for adaptive sqrt(3).\n\n"
       << "-U\n\tRefine mesh uniformly (simulates uniform subdivision).\n\n";

  exit(xcode);    
}

std::string basename(const std::string& _f)
{
  std::string::size_type dot = _f.rfind("/");
  if (dot == std::string::npos)
    return _f;
  return _f.substr(dot+1, _f.length()-(dot+1));
}

// ----------------------------------------------------------------------------
//                                end of file
// ============================================================================

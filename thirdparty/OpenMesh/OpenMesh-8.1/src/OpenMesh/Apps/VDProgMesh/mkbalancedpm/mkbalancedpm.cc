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



// -------------------- STL
#include <iostream>
#include <sstream>
#include <cmath>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Utils/getopt.h>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
#include <OpenMesh/Tools/Decimater/ModNormalFlippingT.hh>
#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
#include <OpenMesh/Tools/Decimater/ModIndependentSetsT.hh>

typedef OpenMesh::TriMesh_ArrayKernelT<> Mesh;
typedef OpenMesh::Decimater::DecimaterT<Mesh> DecimaterProgMesh;
typedef OpenMesh::Decimater::ModNormalFlippingT<Mesh>  ModNormalFlipping;
typedef OpenMesh::Decimater::ModQuadricT<Mesh>         ModQuadric;
typedef OpenMesh::Decimater::ModProgMeshT<Mesh>        ModProgMesh;
typedef OpenMesh::Decimater::ModIndependentSetsT<Mesh> ModIndependentSets;

// ----------------------------------------------------------------------------

using namespace OpenMesh::Decimater;

template <class D>
class ModBalancerT : public OpenMesh::Decimater::ModQuadricT<D>
{
public:

  typedef OpenMesh::Decimater::ModQuadricT<D> BaseModQ;

  DECIMATING_MODULE( ModBalancerT, D, Balancer );

public:

  typedef size_t level_t;

public:

   /// Constructor
  explicit ModBalancerT( D &_dec )
    : BaseModQ( _dec ),
      max_level_(0), n_roots_(0), n_vertices_(0)
  {
    BaseModQ::mesh().add_property( level_ );
  }


  /// Destructor
  virtual ~ModBalancerT()
  {
    BaseModQ::mesh().remove_property( level_ );
  }

public:

  static level_t calc_bits_for_roots( size_t _n_vertices )
  {
    return level_t(std::ceil(std::log((double)_n_vertices)*inv_log2_));
  }

public: // inherited

  void initialize(void) override
  {
    BaseModQ::initialize();
    n_vertices_ = BaseModQ::mesh().n_vertices();
    n_roots_    = calc_bits_for_roots(n_vertices_);
  }

  virtual float collapse_priority(const CollapseInfo& _ci) override
  {
    level_t newlevel = std::max( BaseModQ::mesh().property( level_, _ci.v0 ),
                                 BaseModQ::mesh().property( level_, _ci.v1 ) )+1;
    level_t newroots = calc_bits_for_roots(n_vertices_-1);

    if ( (newroots + newlevel) < 32 )
    {
      double err = BaseModQ::collapse_priority( _ci );

      if (err!=BaseModQ::ILLEGAL_COLLAPSE)
      {
        return float(newlevel + err/(err+1.0));
      }


    }
    return BaseModQ::ILLEGAL_COLLAPSE;
  }

  /// post-process halfedge collapse (accumulate quadrics)
  void postprocess_collapse(const CollapseInfo& _ci) override
  {
    BaseModQ::postprocess_collapse( _ci );

    BaseModQ::mesh().property( level_, _ci.v1 ) =
      std::max( BaseModQ::mesh().property( level_, _ci.v0 ),
		BaseModQ::mesh().property( level_, _ci.v1 ) ) + 1;

    max_level_ = std::max( BaseModQ::mesh().property( level_, _ci.v1 ), max_level_ );
    n_roots_   = calc_bits_for_roots(--n_vertices_);
  }

public:

  level_t max_level(void) const       { return max_level_; }
  level_t bits_for_roots(void) const  { return n_roots_; }

private:

  /// hide this method
  void set_binary(bool _b) {}

  OpenMesh::VPropHandleT<level_t> level_;

  level_t max_level_; // maximum level reached
  level_t n_roots_;   // minimum bits for root nodes
  size_t  n_vertices_;// number of potential root nodes

  static const double inv_log2_;

};

template <typename D>
const double ModBalancerT<D>::inv_log2_ = 1.0/std::log(2.0);

typedef ModBalancerT<Mesh>  ModBalancer;


// ----------------------------------------------------------------------------

inline
std::string&
replace_extension( std::string& _s, const std::string& _e )
{
  std::string::size_type dot = _s.rfind(".");
  if (dot == std::string::npos)
  { _s += "." + _e; }
  else
  { _s = _s.substr(0,dot+1)+_e; }
  return _s;
}

inline
std::string
basename(const std::string& _f)
{
  std::string::size_type dot = _f.rfind("/");
  if (dot == std::string::npos)
    return _f;
  return _f.substr(dot+1, _f.length()-(dot+1));
}

// ----------------------------------------------------------------------------

void usage_and_exit(int xcode)
{
  using namespace std;

  cout << endl
       << "Usage: mkbalancedpm [-n <decimation-steps>] [-o <output>] [-N <max. normal deviation>]"
       << "<input.ext>\n"
       << endl
       << "  Create a balanced progressive mesh from an input file.\n"
       << "  By default decimate as much as possible and write the result\n"
       << "  to <input>.pm\n"
       << endl
       << "Options:\n"
       << endl
       << "  -n <decimation-steps>\n"
       << "\tDetermines the maximum number of decimation steps.\n"
       << "\tDecimate as much as possible if the value is equal zero\n"
       << "\tDefault value: 0\n"
       << endl
       << "  -o <output>\n"
       << "\tWrite resulting progressive mesh to the file named <output>\n"
       << endl
       << "  -N <max. normal Deviation>\n"
       << "\tEnable Normal Flipping\n"
       << endl
       << "  -I\n"
       << "\tEnable Independent Sets\n"
       << endl;    
  exit(xcode);
}

// ----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  Mesh mesh;

  int  c;
  std::string ifname, ofname;
  size_t      decstep=0;
  float       normalDev=90.0;
  bool enable_modNF = false;
  bool enable_modIS = false;

  while ((c=getopt(argc, argv, "n:o:N:Ih"))!=-1)
  {
    switch (c)
    {
      case 'o': ofname = optarg; break;
      case 'n': { std::stringstream str; str << optarg; str >> decstep; } break;
      case 'N': { enable_modNF = true; 
                  std::stringstream str; str << optarg; str >> normalDev; } break;
      case 'I': enable_modIS = true; break;
      case 'h':
        usage_and_exit(0);
        break;
      default:
	usage_and_exit(1);
    }
  }

  if (optind >= argc)
    usage_and_exit(1);   

  ifname = argv[optind];

  if (!OpenMesh::IO::read_mesh(mesh, ifname))
  {
    std::cerr << "Error loading mesh from file '" << ifname << "'!\n";
    return 1;
  }

  {
    OpenMesh::Utils::Timer t;

    DecimaterProgMesh decimater(mesh);

    ModProgMesh::Handle        modPM;
    ModBalancer::Handle        modB;
    ModNormalFlipping::Handle  modNF;
    ModIndependentSets::Handle modIS;

    
    decimater.add(modPM);
    std::cout << "w/  progressive mesh module\n";
    decimater.add(modB);
    std::cout << "w/  balancer module\n";

    if ( enable_modNF )
    {
      decimater.add(modNF);
      decimater.module(modNF).set_max_normal_deviation(normalDev);
    }
    std::cout << "w/" << (modNF.is_valid() ? ' ' : 'o')
              << " normal flipping module (max. normal deviation: " << normalDev << ")\n";

    if ( enable_modIS )
      decimater.add(modIS);
    std::cout << "w/" << (modIS.is_valid() ? ' ' : 'o')
              << " independent sets module\n";

    std::cout << "Initialize decimater\n";
    t.start();
    if ( !decimater.initialize() )
    {
      std::cerr << "  Initialization failed!\n";
      return 1;
    }
    t.stop();
    std::cout << "  done [" << t.as_string() << "]\n";
    t.reset();

    size_t rc;
    size_t nv = mesh.n_vertices();

    std::cout << "Begin decimation (#V " << nv << ")\n";
    t.start();
    do 
    {
      if (modIS.is_valid())
      {
        Mesh::VertexIter  v_it;
        Mesh::FaceIter    f_it;

        for (f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it)
          if ( !mesh.status(*f_it).deleted() )
            mesh.update_normal(*f_it);

        for (v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it)
          if ( !mesh.status(*v_it).deleted() )
          {
            mesh.status(*v_it).set_locked(false);
            mesh.update_normal(*v_it);
          }
       
      }

      rc = decimater.decimate(decstep);
      t.stop();
      std::cout << '\r' 
                << (nv-=rc) << " (-" << rc << ")                " << std::flush;
      t.cont();
    } while (rc > 0);
    t.stop();

    std::cout << "\n  done [" << t.as_string() << "]\n";

    std::cout << "Bits for <tree-id, node-id>: <"
              << decimater.module(modB).bits_for_roots() << ", "
              << decimater.module(modB).max_level()  << ">"
              << std::endl;

    std::cout << "Maximum level reached: "
	      << decimater.module(modB).max_level() << std::endl;

    if (ofname == "." || ofname == ".." )
      ofname += "/" + basename(ifname);
    std::string pmfname = ofname.empty() ? ifname : ofname;
    replace_extension(pmfname, "pm");

    std::cout << "Write progressive mesh data to file "
              << pmfname << std::endl;
    decimater.module(modPM).write( pmfname );
  }


  return 0;
}

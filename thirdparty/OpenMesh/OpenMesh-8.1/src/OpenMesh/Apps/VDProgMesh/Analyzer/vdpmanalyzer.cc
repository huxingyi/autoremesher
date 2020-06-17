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



// -------------------------------------------------------------- includes ----

#include <OpenMesh/Core/System/config.h>
// -------------------- STL
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <exception>
#include <cmath>
// -------------------- OpenMesh
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Utils/Endian.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Tools/Utils/getopt.h>
// -------------------- OpenMesh VDPM
#include <OpenMesh/Tools/VDPM/StreamingDef.hh>
#include <OpenMesh/Tools/VDPM/ViewingParameters.hh>
#include <OpenMesh/Tools/VDPM/VHierarchy.hh>
#include <OpenMesh/Tools/VDPM/VFront.hh>

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------

using namespace OpenMesh;

// ----------------------------------------------------------------------------

// using view dependent progressive mesh
using VDPM::Plane3d;
using VDPM::VFront;
using VDPM::VHierarchy;
using VDPM::VHierarchyNode;
using VDPM::VHierarchyNodeIndex;
using VDPM::VHierarchyNodeHandle;
using VDPM::VHierarchyNodeHandleContainer;
using VDPM::ViewingParameters;


// ------------------------------------------------------------- mesh type ----
// Generate an OpenMesh suitable for the analyzing task

struct AnalyzerTraits : public DefaultTraits
{
  VertexTraits
  {
  public:

    VHierarchyNodeHandle vhierarchy_node_handle()
      { return node_handle_; }

    void set_vhierarchy_node_handle(VHierarchyNodeHandle _node_handle)
      { node_handle_ = _node_handle; }

    bool is_ancestor(const VHierarchyNodeIndex &_other)
      { return false; }

  private:

    VHierarchyNodeHandle  node_handle_;

  };


  HalfedgeTraits
  {
  public:

    VHierarchyNodeHandle
      vhierarchy_leaf_node_handle()
      { return  leaf_node_handle_; }

    void
      set_vhierarchy_leaf_node_handle(VHierarchyNodeHandle _leaf_node_handle)
      { leaf_node_handle_ = _leaf_node_handle; }

  private:

    VHierarchyNodeHandle  leaf_node_handle_;

  };

  VertexAttributes(Attributes::Status |
		   Attributes::Normal);
  HalfedgeAttributes(Attributes::PrevHalfedge);
  EdgeAttributes(Attributes::Status);
  FaceAttributes(Attributes::Status |
		 Attributes::Normal);

};

typedef TriMesh_ArrayKernelT<AnalyzerTraits>  Mesh;

// ----------------------------------------------------------------- types ----

struct PMInfo
{
  Mesh::Point		p0;
  Mesh::VertexHandle	v0, v1, vl, vr;
};

typedef std::vector<PMInfo>                       PMInfoContainer;
typedef PMInfoContainer::iterator                 PMInfoIter;
typedef std::vector<VertexHandle>       VertexHandleContainer;
typedef std::vector<Vec3f>              ResidualContainer;


// -------------------------------------------------------------- forwards ----

/// open progressive mesh
void open_prog_mesh(const std::string &_filename);

/// save view-dependent progressive mesh
void save_vd_prog_mesh(const std::string &_filename);


/// locate fundamental cut vertices
void locate_fund_cut_vertices();

void create_vertex_hierarchy();


/// refine mesh up to _n vertices
void refine(unsigned int _n);

/// coarsen mesh down to _n vertices
void coarsen(unsigned int _n);

void vdpm_analysis();

void get_leaf_node_handles(VHierarchyNodeHandle node_handle,
			   VHierarchyNodeHandleContainer &leaf_nodes);
void compute_bounding_box(VHierarchyNodeHandle node_handle,
			  VHierarchyNodeHandleContainer &leaf_nodes);
void compute_cone_of_normals(VHierarchyNodeHandle node_handle,
			     VHierarchyNodeHandleContainer &leaf_nodes);
void compute_screen_space_error(VHierarchyNodeHandle node_handle,
				VHierarchyNodeHandleContainer &leaf_nodes);
void compute_mue_sigma(VHierarchyNodeHandle node_handle,
		       ResidualContainer &residuals);

Vec3f
point2triangle_residual(const Vec3f &p,
                        const Vec3f tri[3], float &s, float &t);


void PrintOutFundCuts();
void PrintVertexNormals();

// --------------------------------------------------------------- globals ----
// mesh data

Mesh                  mesh_;
PMInfoContainer       pminfos_;
PMInfoIter            pmiter_;

VHierarchy            vhierarchy_;

unsigned int          n_base_vertices_, n_base_faces_, n_details_;
unsigned int          n_current_res_;
unsigned int          n_max_res_;
bool                  verbose = false;


// ----------------------------------------------------------------------------

void usage_and_exit(int xcode)
{
  using namespace std;

  cout << "Usage: vdpmanalyzer [-h] [-o output.spm] input.pm\n";

  exit(xcode);
}

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

// ----------------------------------------------------------------------------

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
// just for debugging

typedef std::vector<OpenMesh::Vec3f>  MyPoints;
typedef MyPoints::iterator            MyPointsIter;

MyPoints  projected_points;
MyPoints  original_points;


// ------------------------------------------------------------------ main ----


int main(int argc, char **argv)
{
  int           c;
  std::string   ifname;
  std::string   ofname;

  while ( (c=getopt(argc, argv, "o:"))!=-1 )
  {
    switch(c)
    {
      case 'v': verbose = true; break;
      case 'o': ofname = optarg;  break;
      case 'h': usage_and_exit(0); break;
      default:  usage_and_exit(1);
    }
  }

  if (optind >= argc)
    usage_and_exit(1);

  ifname = argv[optind];

  if (ofname == "." || ofname == ".." )
    ofname += "/" + basename(ifname);
  std::string spmfname = ofname.empty() ? ifname : ofname;
  replace_extension(spmfname, "spm");

  if ( ifname.empty() || spmfname.empty() )
  {
    usage_and_exit(1);
  }

  try
  {
    open_prog_mesh(ifname);
    vdpm_analysis();
    save_vd_prog_mesh(spmfname);
  }
  catch( std::bad_alloc& )
  {
    std::cerr << "Error: out of memory!\n" << std::endl;
    return 1;
  }
  catch( std::exception& x )
  {
    std::cerr << "Error: " << x.what() << std::endl;
    return 1;
  }
  catch( ... )
  {
    std::cerr << "Fatal! Unknown error!\n";
    return 1;
  }
  return 0;
}


// ----------------------------------------------------------------------------

void 
open_prog_mesh(const std::string& _filename)
{
  Mesh::Point           p;
  unsigned int          i, i0, i1, i2;
  unsigned int          v1, vl, vr;
  char                  c[10];
  VertexHandle          vertex_handle;
  VHierarchyNodeHandle  node_handle, lchild_handle, rchild_handle;
  VHierarchyNodeIndex   node_index;

  std::ifstream  ifs(_filename.c_str(), std::ios::binary);
  if (!ifs)
  {
    std::cerr << "read error\n";
    exit(1);
  }

  //
  bool swap = Endian::local() != Endian::LSB;

  // read header
  ifs.read(c, 8); c[8] = '\0';
  if (std::string(c) != std::string("ProgMesh"))
  {
    std::cerr << "Wrong file format.\n";
    ifs.close();
    exit(1);
  }
  IO::restore(ifs, n_base_vertices_, swap);
  IO::restore(ifs, n_base_faces_, swap);
  IO::restore(ifs, n_details_, swap);

  vhierarchy_.set_num_roots(n_base_vertices_);

  for (i=0; i<n_base_vertices_; ++i)
  {
    IO::restore(ifs, p, swap);

    vertex_handle = mesh_.add_vertex(p);
    node_index    = vhierarchy_.generate_node_index(i, 1);
    node_handle   = vhierarchy_.add_node();

    vhierarchy_.node(node_handle).set_index(node_index);
    vhierarchy_.node(node_handle).set_vertex_handle(vertex_handle);
    mesh_.data(vertex_handle).set_vhierarchy_node_handle(node_handle);
  }

  for (i=0; i<n_base_faces_; ++i)
  {
    IO::restore(ifs, i0, swap);
    IO::restore(ifs, i1, swap);
    IO::restore(ifs, i2, swap);
    mesh_.add_face(mesh_.vertex_handle(i0),
		   mesh_.vertex_handle(i1),
		   mesh_.vertex_handle(i2));
  }

  // load progressive detail
  for (i=0; i<n_details_; ++i)
  {
    IO::restore(ifs, p, swap);
    IO::restore(ifs, v1, swap);
    IO::restore(ifs, vl, swap);
    IO::restore(ifs, vr, swap);

    PMInfo pminfo;
    pminfo.p0 = p;
    pminfo.v0 = mesh_.add_vertex(p);
    pminfo.v1 = Mesh::VertexHandle(v1);
    pminfo.vl = Mesh::VertexHandle(vl);
    pminfo.vr = Mesh::VertexHandle(vr);
    pminfos_.push_back(pminfo);

    node_handle = mesh_.data(pminfo.v1).vhierarchy_node_handle();

    vhierarchy_.make_children(node_handle);
    lchild_handle = vhierarchy_.lchild_handle(node_handle);
    rchild_handle = vhierarchy_.rchild_handle(node_handle);

    mesh_.data(pminfo.v0).set_vhierarchy_node_handle(lchild_handle);
    mesh_.data(pminfo.v1).set_vhierarchy_node_handle(rchild_handle);
    vhierarchy_.node(lchild_handle).set_vertex_handle(pminfo.v0);
    vhierarchy_.node(rchild_handle).set_vertex_handle(pminfo.v1);
  }

  ifs.close();


  // recover mapping between basemesh vertices to roots of vertex hierarchy
  for (i=0; i<n_base_vertices_; ++i)
  {
    node_handle = vhierarchy_.root_handle(i);
    vertex_handle = vhierarchy_.node(node_handle).vertex_handle();
    
    mesh_.data(vertex_handle).set_vhierarchy_node_handle(node_handle);
  }

  pmiter_ = pminfos_.begin();
  n_current_res_ = 0;
  n_max_res_ = n_details_;
 

  // update face and vertex normals
  mesh_.update_face_normals();
  mesh_.update_vertex_normals();   

  // bounding box
  Mesh::ConstVertexIter
    vIt(mesh_.vertices_begin()), 
    vEnd(mesh_.vertices_end());

  Mesh::Point bbMin, bbMax;

  bbMin = bbMax = mesh_.point(*vIt);
  for (; vIt!=vEnd; ++vIt)
  {
    bbMin.minimize(mesh_.point(*vIt));
    bbMax.maximize(mesh_.point(*vIt));
  }

  // info
  std::cerr << mesh_.n_vertices() << " vertices, "
	    << mesh_.n_edges()    << " edge, "
	    << mesh_.n_faces()    << " faces, "
	    << n_details_ << " detail vertices\n";
}


// ----------------------------------------------------------------------------

void
save_vd_prog_mesh(const std::string &_filename)
{
  unsigned i;
  int fvi[3];
  Mesh::Point           p;
  Vec3f                 normal;
  float                 radius, sin_square, mue_square, sigma_square;
  Mesh::FaceIter        f_it;
  Mesh::HalfedgeHandle  hh;
  Mesh::VertexHandle    vh;
  VHierarchyNodeIndex   node_index;
  VHierarchyNodeIndex   fund_lcut_index, fund_rcut_index;
  VHierarchyNodeHandle  node_handle, lchild_handle, rchild_handle;
  std::map<VertexHandle, unsigned int>  handle2index_map;

  std::ofstream ofs(_filename.c_str(), std::ios::binary);
  if (!ofs)
  {
    std::cerr << "write error\n";
    exit(1);
  }
  
  //
  bool swap = Endian::local() != Endian::LSB;

  // write header
  ofs << "VDProgMesh";

  IO::store(ofs, n_base_vertices_, swap);
  IO::store(ofs, n_base_faces_, swap);
  IO::store(ofs, n_details_, swap);

  // write base mesh  
  coarsen(0);
  mesh_.garbage_collection( false, true, true );

  for (i=0; i<n_base_vertices_; ++i)
  {
    node_handle = vhierarchy_.root_handle(i);
    vh = vhierarchy_.node(node_handle).vertex_handle();
    
    p             = mesh_.point(vh);
    radius        = vhierarchy_.node(node_handle).radius();
    normal        = vhierarchy_.node(node_handle).normal();
    sin_square    = vhierarchy_.node(node_handle).sin_square();
    mue_square    = vhierarchy_.node(node_handle).mue_square();
    sigma_square  = vhierarchy_.node(node_handle).sigma_square();

    IO::store(ofs, p, swap);
    IO::store(ofs, radius, swap);
    IO::store(ofs, normal, swap);
    IO::store(ofs, sin_square, swap);
    IO::store(ofs, mue_square, swap);
    IO::store(ofs, sigma_square, swap);

    handle2index_map[vh] = i;
  }

 
  for (f_it=mesh_.faces_begin(); f_it!=mesh_.faces_end(); ++f_it) {
    hh = mesh_.halfedge_handle(*f_it);
    vh = mesh_.to_vertex_handle(hh);
    fvi[0] = handle2index_map[vh];

    hh = mesh_.next_halfedge_handle(hh);
    vh = mesh_.to_vertex_handle(hh);
    fvi[1] = handle2index_map[vh];

    hh = mesh_.next_halfedge_handle(hh);
    vh = mesh_.to_vertex_handle(hh);
    fvi[2] = handle2index_map[vh];

    IO::store(ofs, fvi[0], swap);
    IO::store(ofs, fvi[1], swap);
    IO::store(ofs, fvi[2], swap);
  }


  // write progressive detail (vertex hierarchy)

  for (i=0; i<n_details_; ++i)
  {
    PMInfo  pminfo = *pmiter_;

    p = mesh_.point(pminfo.v0);

    IO::store(ofs, p, swap);


    node_handle   = mesh_.data(pminfo.v1).vhierarchy_node_handle();
    lchild_handle = vhierarchy_.lchild_handle(node_handle);
    rchild_handle = vhierarchy_.rchild_handle(node_handle);

    node_index      = vhierarchy_.node(node_handle).node_index();
    fund_lcut_index = vhierarchy_.node(node_handle).fund_lcut_index();
    fund_rcut_index = vhierarchy_.node(node_handle).fund_rcut_index();

    IO::store(ofs, node_index.value(), swap);
    IO::store(ofs, fund_lcut_index.value(), swap);
    IO::store(ofs, fund_rcut_index.value(), swap);

    radius        = vhierarchy_.node(lchild_handle).radius();
    normal        = vhierarchy_.node(lchild_handle).normal();
    sin_square    = vhierarchy_.node(lchild_handle).sin_square();
    mue_square    = vhierarchy_.node(lchild_handle).mue_square();
    sigma_square  = vhierarchy_.node(lchild_handle).sigma_square();

    IO::store(ofs, radius, swap);
    IO::store(ofs, normal, swap);
    IO::store(ofs, sin_square, swap);
    IO::store(ofs, mue_square, swap);
    IO::store(ofs, sigma_square, swap);

    radius        = vhierarchy_.node(rchild_handle).radius();
    normal        = vhierarchy_.node(rchild_handle).normal();
    sin_square    = vhierarchy_.node(rchild_handle).sin_square();
    mue_square    = vhierarchy_.node(rchild_handle).mue_square();
    sigma_square  = vhierarchy_.node(rchild_handle).sigma_square();

    IO::store(ofs, radius, swap);
    IO::store(ofs, normal, swap);
    IO::store(ofs, sin_square, swap);
    IO::store(ofs, mue_square, swap);
    IO::store(ofs, sigma_square, swap);

    refine(i);
  }
  
  ofs.close();

  std::cout << "save view-dependent progressive mesh" << std::endl;
}

//-----------------------------------------------------------------------------

void refine(unsigned int _n)
{
  while (n_current_res_ < _n && pmiter_ != pminfos_.end())
  {
    mesh_.vertex_split(pmiter_->v0,
		       pmiter_->v1,
		       pmiter_->vl,
		       pmiter_->vr);

    VHierarchyNodeHandle
      parent_handle = mesh_.data(pmiter_->v1).vhierarchy_node_handle();

    VHierarchyNodeHandle  
      lchild_handle = vhierarchy_.lchild_handle(parent_handle),
      rchild_handle = vhierarchy_.rchild_handle(parent_handle);

    mesh_.data(pmiter_->v0).set_vhierarchy_node_handle(lchild_handle);
    mesh_.data(pmiter_->v1).set_vhierarchy_node_handle(rchild_handle);

    ++pmiter_;
    ++n_current_res_;
  }
}


//-----------------------------------------------------------------------------


void coarsen(unsigned int _n)
{
  while (n_current_res_ > _n && pmiter_ != pminfos_.begin()) 
  {
    --pmiter_;

    Mesh::HalfedgeHandle hh = 
      mesh_.find_halfedge(pmiter_->v0, pmiter_->v1);
    mesh_.collapse(hh);

    VHierarchyNodeHandle  
      rchild_handle = mesh_.data(pmiter_->v1).vhierarchy_node_handle();

    VHierarchyNodeHandle
      parent_handle = vhierarchy_.parent_handle(rchild_handle);

    mesh_.data(pmiter_->v1).set_vhierarchy_node_handle(parent_handle);
    
    --n_current_res_;
  }
}


//-----------------------------------------------------------------------------


void 
vdpm_analysis()
{
  unsigned int                    i;
  Mesh::VertexHandle            vh;
  Mesh::VertexIter              v_it;
  Mesh::HalfedgeIter            h_it;
  Mesh::HalfedgeHandle          h, o, hn, op, hpo, on, ono;
  VHierarchyNodeHandleContainer   leaf_nodes;
 
  OpenMesh::Utils::Timer tana;
  tana.start();
 
  refine(n_max_res_);
  
  mesh_.update_face_normals();
  mesh_.update_vertex_normals();

  std::cout << "Init view-dependent PM analysis" << std::endl;
  
  // initialize
  for (h_it=mesh_.halfedges_begin(); h_it!=mesh_.halfedges_end(); ++h_it)
  {
    vh = mesh_.to_vertex_handle(*h_it);
    mesh_.data(*h_it).set_vhierarchy_leaf_node_handle(mesh_.data(vh).vhierarchy_node_handle());
  }

  for (v_it=mesh_.vertices_begin(); v_it!=mesh_.vertices_end(); ++v_it)
  {
    VHierarchyNodeHandle  
      node_handle = mesh_.data(*v_it).vhierarchy_node_handle();
    
    vhierarchy_.node(node_handle).set_normal(mesh_.normal(*v_it));
  }
  
  std::cout << "Start view-dependent PM analysis" << std::endl;

  // locate fundamental cut vertices in each edge collapse
  OpenMesh::Utils::Timer t;

  for (i=n_max_res_; i>0; --i)
  {
    t.start();
    PMInfo   pminfo   = pminfos_[i-1];

    if (verbose)
      std::cout << "Analyzing " << i << "-th detail vertex" << std::endl;

    // maintain leaf node pointers & locate fundamental cut vertices
    h   = mesh_.find_halfedge(pminfo.v0, pminfo.v1);
    o   = mesh_.opposite_halfedge_handle(h);
    hn  = mesh_.next_halfedge_handle(h);
    hpo = mesh_.opposite_halfedge_handle(mesh_.prev_halfedge_handle(h));
    op  = mesh_.prev_halfedge_handle(o);
    on  = mesh_.next_halfedge_handle(o);
    ono = mesh_.opposite_halfedge_handle(on);

    VHierarchyNodeHandle  
      rchild_handle = mesh_.data(pminfo.v1).vhierarchy_node_handle();

    VHierarchyNodeHandle
      parent_handle = vhierarchy_.parent_handle(rchild_handle);
    
    if (pminfo.vl != Mesh::InvalidVertexHandle)
    {
      VHierarchyNodeHandle  
      fund_lcut_handle  = mesh_.data(hn).vhierarchy_leaf_node_handle();

      VHierarchyNodeHandle
        left_leaf_handle  = mesh_.data(hpo).vhierarchy_leaf_node_handle();

      mesh_.data(hn).set_vhierarchy_leaf_node_handle(left_leaf_handle);

      vhierarchy_.node(parent_handle).
	set_fund_lcut(vhierarchy_.node_index(fund_lcut_handle));
    }

    if (pminfo.vr != Mesh::InvalidVertexHandle)
    {
      VHierarchyNodeHandle
	fund_rcut_handle  = mesh_.data(on).vhierarchy_leaf_node_handle(),
	right_leaf_handle = mesh_.data(ono).vhierarchy_leaf_node_handle();

      mesh_.data(op).set_vhierarchy_leaf_node_handle(right_leaf_handle);

      vhierarchy_.node(parent_handle).
	set_fund_rcut(vhierarchy_.node_index(fund_rcut_handle));
    }

    coarsen(i-1);
    
    leaf_nodes.clear();

    get_leaf_node_handles(parent_handle, leaf_nodes);
    compute_bounding_box(parent_handle, leaf_nodes);
    compute_cone_of_normals(parent_handle, leaf_nodes);
    compute_screen_space_error(parent_handle, leaf_nodes);

    t.stop();

    if (verbose)
    {
      std::cout << "  radius of bounding sphere: "
                << vhierarchy_.node(parent_handle).radius() << std::endl;
      std::cout << "  direction of cone of normals: " 
                << vhierarchy_.node(parent_handle).normal() << std::endl;
      std::cout << "  sin(semi-angle of cone of normals) ^2: " 
                << vhierarchy_.node(parent_handle).sin_square() << std::endl;
      std::cout << "  (mue^2, sigma^2) : (" 
                << vhierarchy_.node(parent_handle).mue_square()   << ", " 
                << vhierarchy_.node(parent_handle).sigma_square() << ")" 
                << std::endl;
      std::cout << "- " << t.as_string() << std::endl;
    }

  } // end for all collapses

  tana.stop();
  std::cout << "Analyzing step completed in " 
            << tana.as_string() << std::endl;
}


// ----------------------------------------------------------------------------

void 
get_leaf_node_handles(VHierarchyNodeHandle           node_handle, 
		      VHierarchyNodeHandleContainer &leaf_nodes)
{
  if (vhierarchy_.node(node_handle).is_leaf())
  {
    leaf_nodes.push_back(node_handle);
  }
  else
  {
    get_leaf_node_handles(vhierarchy_.node(node_handle).lchild_handle(), 
			  leaf_nodes);
    get_leaf_node_handles(vhierarchy_.node(node_handle).rchild_handle(), 
			  leaf_nodes);
  }
}


// ----------------------------------------------------------------------------

void 
compute_bounding_box(VHierarchyNodeHandle node_handle, VHierarchyNodeHandleContainer &leaf_nodes)
{
  float             max_distance;
  Vec3f   p, lp;
  VHierarchyNodeHandleContainer::iterator   n_it, n_end(leaf_nodes.end());

  max_distance = 0.0f;
  VertexHandle  vh = vhierarchy_.node(node_handle).vertex_handle();
  p = mesh_.point(vh);
  for ( n_it = leaf_nodes.begin(); n_it != n_end; ++n_it )
  {
    lp = mesh_.point(vhierarchy_.vertex_handle(*n_it));
    max_distance = std::max(max_distance, (p - lp).length());
  }

  vhierarchy_.node(node_handle).set_radius(max_distance);
}


// ----------------------------------------------------------------------------

void
compute_cone_of_normals(VHierarchyNodeHandle           node_handle,
			VHierarchyNodeHandleContainer &leaf_nodes)
{
  Vec3f           n, ln;
  VertexHandle              vh = vhierarchy_.node(node_handle).vertex_handle();
  VHierarchyNodeHandleContainer::iterator  n_it, n_end(leaf_nodes.end());

  n               = mesh_.calc_vertex_normal(vh);
  float max_angle = 0.0f;

  n_it = leaf_nodes.begin();
  while( n_it != n_end )
  {
    ln        = vhierarchy_.node(*n_it).normal();
    const float angle     = acosf( dot(n,ln) );
    max_angle = std::max(max_angle, angle );

    ++n_it;
  }

  max_angle = std::min(max_angle, float(M_PI_2));
  mesh_.set_normal(vh, n);
  vhierarchy_.node(node_handle).set_normal(n);
  vhierarchy_.node(node_handle).set_semi_angle(max_angle);
}


// ----------------------------------------------------------------------------

void
compute_screen_space_error(VHierarchyNodeHandle node_handle, VHierarchyNodeHandleContainer &leaf_nodes)
{
  std::vector<Vec3f>    residuals;
  Mesh::VertexFaceIter  vf_it;
  Mesh::HalfedgeHandle  heh;
  Mesh::VertexHandle    vh;
  Vec3f                 residual;
  Vec3f                 res;
  Vec3f                 lp;
#if ((defined(_MSC_VER) && (_MSC_VER >= 1800))  )
  // Workaround for internal compiler error
  Vec3f                 tri[3]{ {},{},{} };
#else
  Vec3f                 tri[3];
#endif
  float                 s, t;
  VHierarchyNodeHandleContainer::iterator  n_it, n_end(leaf_nodes.end());

  for ( n_it = leaf_nodes.begin(); n_it != n_end; ++n_it )
  {
    lp = mesh_.point(vhierarchy_.node(*n_it).vertex_handle());

    // compute residual of a leaf-vertex from the current mesh_
    vh = vhierarchy_.node(node_handle).vertex_handle();
    residual = lp - mesh_.point(vh);
    float min_distance = residual.length();

    for (vf_it=mesh_.vf_iter(vh); vf_it.is_valid(); ++vf_it)
    {
      heh    = mesh_.halfedge_handle(*vf_it);
      tri[0] = mesh_.point(mesh_.to_vertex_handle(heh));
      heh    = mesh_.next_halfedge_handle(heh);
      tri[1] = mesh_.point(mesh_.to_vertex_handle(heh));
      heh    = mesh_.next_halfedge_handle(heh);
      tri[2] = mesh_.point(mesh_.to_vertex_handle(heh));

      res = point2triangle_residual(lp, tri, s, t);

      if (res.length() < min_distance)
			{
				residual = res;
				min_distance = res.length();
			}
    }

    residuals.push_back(residual);
  }

  compute_mue_sigma(node_handle, residuals);
}


// ----------------------------------------------------------------------------

void
compute_mue_sigma(VHierarchyNodeHandle node_handle,
		  ResidualContainer &residuals)
{
  Vec3f   vn;
  float             max_inner, max_cross;
  ResidualContainer::iterator  r_it, r_end(residuals.end());

  max_inner = max_cross = 0.0f;
  vn = mesh_.normal(vhierarchy_.node(node_handle).vertex_handle());
  for (r_it = residuals.begin(); r_it != r_end; ++r_it)
  {
    float inner = fabsf(dot(*r_it, vn));
    float cross = OpenMesh::cross(*r_it, vn).length();

    max_inner = std::max(max_inner, inner);
    max_cross = std::max(max_cross, cross);
  }

  if (max_cross < 1.0e-7)
  {
    vhierarchy_.node(node_handle).set_mue(max_cross);
    vhierarchy_.node(node_handle).set_sigma(max_inner);
  }
  else {
    float  ratio = std::max(1.0f, max_inner/max_cross);
    float  whole_degree = acosf(1.0f/ratio);
    float  mue, max_mue;
    Vec3f  res;

    max_mue = 0.0f;
    for (r_it = residuals.begin(); r_it != r_end; ++r_it)
    {
      res = *r_it;
      float res_length = res.length();

      // TODO: take care when res.length() is too small
      float degree = acosf(dot(vn,res) / res_length);

      if (degree < 0.0f)    degree = -degree;
      if (degree > float(M_PI_2))  degree = float(M_PI) - degree;

      if (degree < whole_degree)
        mue = cosf(whole_degree - degree) * res_length;
      else
        mue = res_length;

      max_mue = std::max(max_mue, mue);
    }

    vhierarchy_.node(node_handle).set_mue(max_mue);
    vhierarchy_.node(node_handle).set_sigma(ratio*max_mue);
  }
}

// ----------------------------------------------------------------------------


Vec3f
point2triangle_residual(const Vec3f &p, const Vec3f tri[3], float &s, float &t)
{ 
  OpenMesh::Vec3f B = tri[0];             // Tri.Origin();
  OpenMesh::Vec3f E0 = tri[1] - tri[0];   // rkTri.Edge0()
  OpenMesh::Vec3f E1 = tri[2] - tri[0];   // rkTri.Edge1()
  OpenMesh::Vec3f D = tri[0] - p;         // kDiff
  float	a = dot(E0, E0);                  // fA00
  float	b = dot(E0, E1);                  // fA01
  float	c = dot(E1, E1);                  // fA11
  float	d = dot(E0, D);                   // fB0
  float	e = dot(E1, D);                   // fB1
  //float	f = dot(D, D);                    // fC
  float det = fabsf(a*c - b*b);
  s = b*e-c*d;
  t = b*d-a*e;
	
  OpenMesh::Vec3f     residual;

//  float distance2;

  if ( s + t <= det )
  {
    if ( s < 0.0f )
    {
      if ( t < 0.0f )  // region 4
      {
        if ( d < 0.0f )
        {
          t = 0.0f;
          if ( -d >= a )
          {
            s = 1.0f;
//            distance2 = a+2.0f*d+f;
          }
          else
          {
            s = -d/a;
//            distance2 = d*s+f;
          }
        }
        else
        {
          s = 0.0f;
          if ( e >= 0.0f )
          {
            t = 0.0f;
//            distance2 = f;
          }
          else if ( -e >= c )
          {
            t = 1.0f;
//            distance2 = c+2.0f*e+f;
          }
          else
          {
            t = -e/c;
//            distance2 = e*t+f;
          }
        }
      }
      else  // region 3
      {
        s = 0.0f;
        if ( e >= 0.0f )
        {
          t = 0.0f;
//          distance2 = f;
        }
        else if ( -e >= c )
        {
          t = 1.0f;
//          distance2 = c+2.0f*e+f;
        }
        else
        {
          t = -e/c;
//          distance2 = e*t+f;
        }
      }
    }
    else if ( t < 0.0f )  // region 5
    {
      t = 0.0f;
      if ( d >= 0.0f )
      {
        s = 0.0f;
//        distance2 = f;
      }
      else if ( -d >= a )
      {
        s = 1.0f;
//        distance2 = a+2.0f*d+f;
      }
      else
      {
        s = -d/a;
//        distance2 = d*s+f;
      }
    }
    else  // region 0
    {
      // minimum at interior point
      float inv_det = 1.0f/det;
      s *= inv_det;
      t *= inv_det;
//      distance2 = s*(a*s+b*t+2.0f*d) + t*(b*s+c*t+2.0f*e)+f;
    }
  }
  else
  {
    float tmp0, tmp1, numer, denom;

    if ( s < 0.0f )  // region 2
    {
      tmp0 = b + d;
      tmp1 = c + e;
      if ( tmp1 > tmp0 )
      {
        numer = tmp1 - tmp0;
        denom = a-2.0f*b+c;
        if ( numer >= denom )
        {
          s = 1.0f;
          t = 0.0f;
//          distance2 = a+2.0f*d+f;
        }
        else
        {
          s = numer/denom;
          t = 1.0f - s;
//          distance2 = s*(a*s+b*t+2.0f*d) + t*(b*s+c*t+2.0f*e)+f;
        }
      }
      else
      {
        s = 0.0f;
        if ( tmp1 <= 0.0f )
        {
          t = 1.0f;
//          distance2 = c+2.0f*e+f;
        }
        else if ( e >= 0.0f )
        {
          t = 0.0f;
//          distance2 = f;
        }
        else
        {
          t = -e/c;
//          distance2 = e*t+f;
        }
      }
    }
    else if ( t < 0.0f )  // region 6
    {
      tmp0 = b + e;
      tmp1 = a + d;
      if ( tmp1 > tmp0 )
      {
        numer = tmp1 - tmp0;
        denom = a-2.0f*b+c;
        if ( numer >= denom )
        {
          t = 1.0f;
          s = 0.0f;
//          distance2 = c+2.0f*e+f;
        }
        else
        {
          t = numer/denom;
          s = 1.0f - t;
//          distance2 = s*(a*s+b*t+2.0f*d)+ t*(b*s+c*t+2.0f*e)+f;
        }
      }
      else
      {
        t = 0.0f;
        if ( tmp1 <= 0.0f )
        {
          s = 1.0f;
//          distance2 = a+2.0f*d+f;
        }
        else if ( d >= 0.0f )
        {
          s = 0.0f;
//          distance2 = f;
        }
        else
        {
          s = -d/a;
//          distance2 = d*s+f;
        }
      }
    }
    else  // region 1
    {
      numer = c + e - b - d;
      if ( numer <= 0.0f )
      {
        s = 0.0f;
        t = 1.0f;
//        distance2 = c+2.0f*e+f;
      }
      else
      {
        denom = a-2.0f*b+c;
        if ( numer >= denom )
        {
          s = 1.0f;
          t = 0.0f;
//          distance2 = a+2.0f*d+f;
        }
        else
        {
          s = numer/denom;
          t = 1.0f - s;
//          distance2 = s*(a*s+b*t+2.0f*d) + t*(b*s+c*t+2.0f*e)+f;
        }
      }
    }
  }

  residual = p - (B + s*E0 + t*E1);

  return	residual;
}

// ============================================================================

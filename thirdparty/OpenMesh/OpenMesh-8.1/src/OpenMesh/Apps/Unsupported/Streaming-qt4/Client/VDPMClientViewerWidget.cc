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




//== INCLUDES =================================================================

#ifdef _MSC_VER
#  pragma warning(disable: 4267 4311)
#endif

#include <iostream>
#include <fstream>
#include <map>

#include <QApplication>
#include <QDateTime>
#include <QFileDialog>
#include <QDataStream>
#include <GL/glut.h>


#include <OpenMesh/Apps/VDProgMesh/Streaming/Client/VDPMClientViewerWidget.hh>
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Utils/Endian.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>

// #include "ImageData.h"


using OpenMesh::VDPM::debug_print;
using OpenMesh::VDPM::set_debug_print;

using OpenMesh::VDPM::VHierarchyNode;
using OpenMesh::VDPM::VHierarchyNodeHandle;
using OpenMesh::VDPM::VHierarchyNodeHandleContainer;



#ifdef EXAMPLE_CREATION

static OpenMesh::Vec3uc myYellow  = OpenMesh::Vec3uc(255, 255, 0);
static OpenMesh::Vec3uc myBlue    = OpenMesh::Vec3uc(0, 0, 255);
std::map<VHierarchyNodeIndex, unsigned int> g_index2numdesc_map;

void VDPMClientViewerWidget::increase_max_descendents(const VHierarchyNodeIndex &_node_index)
{
  g_index2numdesc_map[_node_index] = 2 + g_index2numdesc_map[_node_index];

  unsigned char tree_id_bits = vhierarchy_.tree_id_bits();
  VHierarchyNodeIndex parent_index 
    = VHierarchyNodeIndex(_node_index.tree_id(tree_id_bits), _node_index.node_id(tree_id_bits) / 2, tree_id_bits);

  if (parent_index.is_valid(tree_id_bits) == true)
    increase_max_descendents(parent_index);
}

void VDPMClientViewerWidget::increase_cur_descendents(VHierarchyNodeHandle _node_handle)
{
  unsigned int cur_desc = vhierarchy_.node(_node_handle).cur_descendents();
 
  vhierarchy_.node(_node_handle).set_cur_descendents(2 + cur_desc);

  VHierarchyNodeHandle  parent_handle = vhierarchy_.parent_handle(_node_handle);
  if (parent_handle.is_valid())
    increase_cur_descendents(parent_handle);
}

void VDPMClientViewerWidget::__add_children(const VHierarchyNodeIndex &_node_index, bool update_current)
{
  if (update_current == true)
  {
    increase_cur_descendents(vhierarchy_.node_handle(_node_index));
  }
  else
  {
    unsigned char tree_id_bits = vhierarchy_.tree_id_bits();

    VHierarchyNodeIndex lchild_index 
      = VHierarchyNodeIndex(_node_index.tree_id(tree_id_bits), 2*_node_index.node_id(tree_id_bits), tree_id_bits);
    VHierarchyNodeIndex rchild_index 
      = VHierarchyNodeIndex(_node_index.tree_id(tree_id_bits), 1+2*_node_index.node_id(tree_id_bits), tree_id_bits);

    g_index2numdesc_map[lchild_index] = 1;
    g_index2numdesc_map[rchild_index] = 1;

    increase_max_descendents(_node_index);
  }
}

void VDPMClientViewerWidget::mesh_coloring()
{
  MyMesh::VertexIter  vIt(mesh_.vertices_begin()), 
                      vEnd(mesh_.vertices_end());

  VHierarchyNodeHandle  node_handle;

  for (; vIt!=vEnd; ++vIt)
  {
    node_handle = mesh_.data(*vIt).vhierarchy_node_handle();
    const float ratio = vhierarchy_.node(node_handle).ratio();
    
    const unsigned char r = (unsigned char) ((1.0f - ratio) * myYellow[0] + ratio * myBlue[0]);
    const unsigned char g = (unsigned char) ((1.0f - ratio) * myYellow[1] + ratio * myBlue[1]);
    const unsigned char b = (unsigned char) ((1.0f - ratio) * myYellow[2] + ratio * myBlue[2]);

    mesh_.set_color(*vIt, OpenMesh::Vec3uc(r,g,b));
  }
}

#endif



void
VDPMClientViewerWidget::
draw_scene(const std::string &_draw_mode)
{
  //std::cout << frame_ << "-th frame statistics" << std::endl;

  if (adaptive_mode_ == true)  
  {

    refinement_timer_.start();
    adaptive_refinement();
    refinement_timer_.stop();
    fprintf(refinement_file, "%d %d\n", frame_, (int) refinement_timer_.mseconds());

#ifdef EXAMPLE_CREATION
    mesh_coloring();
#endif
  }

  render_timer_.start();
  MeshViewerWidget::draw_scene(_draw_mode);  
  render_timer_.stop();
  fprintf(render_file, "%d %d %d\n", frame_, (int) render_timer_.mseconds(), mesh_.n_faces());

  ++frame_;
}


void
VDPMClientViewerWidget::
adaptive_refinement()
{
  update_viewing_parameters();

  MyMesh::HalfedgeHandle v0v1;
  float fovy = viewing_parameters_.fovy();
  float tolerance_square = viewing_parameters_.tolerance_square();
	float	tan_value = tanf(fovy / 2.0f);
	kappa_square_ = 4.0f * tan_value * tan_value * tolerance_square;


  for (vfront_.begin(); vfront_.end() != true;)
  {
    VHierarchyNodeHandle  
      node_handle   = vfront_.node_handle(),
      parent_handle = vhierarchy_.parent_handle(node_handle);

    if (qrefine(node_handle) == true)
    {
      if (vhierarchy_.is_leaf_node(node_handle) != true)
      {
        force_vsplit(node_handle);
      }
      else
      {
        //if (qSocket_->bytesAvailable() == 0)
        if (session_running_ != true)
        {
          session_running_ = true;
          send_viewing_information();
        }
        vfront_.next();
      }
    }
    else if (vhierarchy_.is_root_node(node_handle) != true && 
	     ecol_legal(parent_handle, v0v1)       == true && 
	     qrefine(parent_handle) != true)
    {
      ecol(parent_handle, v0v1);
    }
    else
    {
      vfront_.next();
    }
  }
  
  // free memories taged as 'deleted'
  mesh_.garbage_collection(false, true, true); 
  mesh_.update_face_normals();
}


void
VDPMClientViewerWidget::
current_max_resolution()
{
  for (vfront_.begin(); vfront_.end() != true;)
  {
    VHierarchyNodeHandle
      node_handle   = vfront_.node_handle(),
      parent_handle = vhierarchy_.parent_handle(node_handle);

    if (vhierarchy_.is_leaf_node(node_handle) != true)
      force_vsplit(node_handle);
    else
      vfront_.next();
  }

  // free memories taged as 'deleted'
  mesh_.garbage_collection(false, true, true); 
  mesh_.update_face_normals();
}


bool
VDPMClientViewerWidget::
qrefine(VHierarchyNodeHandle _node_handle)
{
  VHierarchyNode &node = vhierarchy_.node(_node_handle);
  OpenMesh::Vec3f p = mesh_.point(node.vertex_handle());
  OpenMesh::Vec3f eye_dir = p - viewing_parameters_.eye_pos();
  float	distance = eye_dir.length();
  float	distance2 = distance * distance;
  float	product_value = dot(eye_dir, node.normal());
  
  if (outside_view_frustum(p, node.radius()) == true)
    return false;
  
  if (oriented_away(node.sin_square(), distance2, product_value) == true)
    return false;
  
  if (screen_space_error(node.mue_square(), 
			 node.sigma_square(), distance2, product_value) == true)
    return false;
  
  return true;
}


void 
VDPMClientViewerWidget::
force_vsplit(VHierarchyNodeHandle node_handle)
{
  MyMesh::VertexHandle  vl, vr;

  get_active_cuts(node_handle, vl, vr);

  while (vl == vr) {
    force_vsplit(mesh_.data(vl).vhierarchy_node_handle());
    get_active_cuts(node_handle, vl, vr);
  }
  
  vsplit(node_handle, vl, vr);
}



void
VDPMClientViewerWidget::
vsplit(VHierarchyNodeHandle _node_handle, MyMesh::VertexHandle vl, MyMesh::VertexHandle vr)
{
  // refine
  VHierarchyNodeHandle  lchild_handle = vhierarchy_.lchild_handle(_node_handle);
  VHierarchyNodeHandle  rchild_handle = vhierarchy_.rchild_handle(_node_handle);
  MyMesh::VertexHandle  v0 = vhierarchy_.vertex_handle(lchild_handle);
  MyMesh::VertexHandle  v1 = vhierarchy_.vertex_handle(rchild_handle);

  mesh_.vertex_split(v0, v1, vl, vr);
  mesh_.set_normal(v0, vhierarchy_.normal(lchild_handle));
  mesh_.set_normal(v1, vhierarchy_.normal(rchild_handle));
  mesh_.data(v0).set_vhierarchy_node_handle(lchild_handle);
  mesh_.data(v1).set_vhierarchy_node_handle(rchild_handle);
  mesh_.status(v0).set_deleted(false);
  mesh_.status(v1).set_deleted(false);

  vfront_.remove(_node_handle);
  vfront_.add(lchild_handle);
  vfront_.add(rchild_handle);  
}


void 
VDPMClientViewerWidget::
ecol(VHierarchyNodeHandle _node_handle, const MyMesh::HalfedgeHandle& v0v1)
{
  VHierarchyNodeHandle  lchild_handle = vhierarchy_.lchild_handle(_node_handle);
  VHierarchyNodeHandle  rchild_handle = vhierarchy_.rchild_handle(_node_handle);
  MyMesh::VertexHandle  v0 = vhierarchy_.vertex_handle(lchild_handle);
  MyMesh::VertexHandle  v1 = vhierarchy_.vertex_handle(rchild_handle);

  // coarsen
  mesh_.collapse(v0v1);
  mesh_.set_normal(v1, vhierarchy_.normal(_node_handle));
  mesh_.data(v0).set_vhierarchy_node_handle(lchild_handle);
  mesh_.data(v1).set_vhierarchy_node_handle(_node_handle);
  mesh_.status(v0).set_deleted(false);
  mesh_.status(v1).set_deleted(false);

  vfront_.add(_node_handle);
  vfront_.remove(lchild_handle);
  vfront_.remove(rchild_handle);
}


bool
VDPMClientViewerWidget::
ecol_legal(VHierarchyNodeHandle _parent_handle, MyMesh::HalfedgeHandle& v0v1)
{
  VHierarchyNodeHandle  lchild_handle = vhierarchy_.lchild_handle(_parent_handle);
  VHierarchyNodeHandle  rchild_handle = vhierarchy_.rchild_handle(_parent_handle);

  // test whether lchild & rchild present in the current vfront
  if (vfront_.is_active(lchild_handle) != true || vfront_.is_active(rchild_handle) != true)
    return  false;  

  MyMesh::VertexHandle v0, v1;
  v0 = vhierarchy_.vertex_handle(lchild_handle);
  v1 = vhierarchy_.vertex_handle(rchild_handle);
  v0v1 = mesh_.find_halfedge(v0, v1);
  
  return  mesh_.is_collapse_ok(v0v1);
}

void
VDPMClientViewerWidget::
get_active_cuts(const VHierarchyNodeHandle _node_handle, MyMesh::VertexHandle &vl, MyMesh::VertexHandle &vr)
{
  MyMesh::VertexVertexIter  vv_it;
  VHierarchyNodeHandle      nnode_handle;
  VHierarchyNodeIndex       nnode_index;
  VHierarchyNodeIndex       fund_lcut_index = vhierarchy_.fund_lcut_index(_node_handle);
  VHierarchyNodeIndex       fund_rcut_index = vhierarchy_.fund_rcut_index(_node_handle);

  vl = MyMesh::InvalidVertexHandle;
	vr = MyMesh::InvalidVertexHandle;
  
  for (vv_it=mesh_.vv_iter(vhierarchy_.vertex_handle(_node_handle)); vv_it; ++vv_it)
	{
    nnode_handle = mesh_.data(*vv_it).vhierarchy_node_handle();
    nnode_index = vhierarchy_.node_index(nnode_handle);

    if (vl == MyMesh::InvalidVertexHandle && vhierarchy_.is_ancestor(nnode_index, fund_lcut_index) == true)
      vl = *vv_it;
    if (vr == MyMesh::InvalidVertexHandle && vhierarchy_.is_ancestor(nnode_index, fund_rcut_index) == true)
      vr = *vv_it;

    /*if (vl == MyMesh::InvalidVertexHandle && nnode_index.is_ancestor_index(fund_lcut_index) == true)
      vl = *vv_it;
    if (vr == MyMesh::InvalidVertexHandle && nnode_index.is_ancestor_index(fund_rcut_index) == true)
      vr = *vv_it;*/

    if (vl != MyMesh::InvalidVertexHandle && vr != MyMesh::InvalidVertexHandle)
      break;
	}
}

bool 
VDPMClientViewerWidget::
outside_view_frustum(const OpenMesh::Vec3f &pos, float radius)
{
  Plane3d   frustum_plane[4];

  viewing_parameters_.frustum_planes(frustum_plane);

	for (int i = 0; i < 4; i++) {
		if (frustum_plane[i].signed_distance(pos) < -radius)
			return	true;
	}
	
	return false;
}

bool 
VDPMClientViewerWidget::
oriented_away(float sin_square, float distance_square, float product_value)
{
	if (product_value > 0 && product_value*product_value > distance_square * sin_square)
		return	true;
	else
		return	false;
}

bool 
VDPMClientViewerWidget::
screen_space_error(float mue_square, float sigma_square, float distance_square, float product_value)
{
	if ((mue_square >= kappa_square_ * distance_square) || 
		(sigma_square * (distance_square - product_value * product_value) >= kappa_square_ * distance_square * distance_square))
		return	false;
	else
		return	true;
}

void
VDPMClientViewerWidget::
open_vd_prog_mesh(const char* _filename)
{
  unsigned int                    i;
  unsigned int                    value;
  unsigned int                    fvi[3];
  char                            fileformat[16];
  OpenMesh::Vec3f                 p, normal;
  float                           radius, sin_square, mue_square, sigma_square;
  VHierarchyNodeHandleContainer   roots;
  OpenMesh::VertexHandle          vertex_handle;  
  VHierarchyNodeIndex             node_index, lchild_node_index, rchild_node_index, fund_lcut_index, fund_rcut_index;
  VHierarchyNodeHandle            node_handle, lchild_handle, rchild_handle;  
  std::map<VHierarchyNodeIndex, VHierarchyNodeHandle> index2handle_map;

  std::ifstream ifs(_filename, std::ios::binary);
  if (!ifs)
  {
    std::cerr << "read error\n";
    exit(1);
  }

  //
  bool swap = OpenMesh::Endian::local() != OpenMesh::Endian::LSB;

  // read header  
  ifs.read(fileformat, 10); fileformat[10] = '\0';
  if (std::string(fileformat) != std::string("VDProgMesh"))
  {
    std::cerr << "Wrong file format.\n";
    ifs.close();
    exit(1);
  }
  OpenMesh::IO::restore(ifs, n_base_vertices_, swap);
  OpenMesh::IO::restore(ifs, n_base_faces_, swap);
  OpenMesh::IO::restore(ifs, n_details_, swap);
  
  mesh_.clear();
  vfront_.clear();
  vhierarchy_.clear();

  vhierarchy_.set_num_roots(n_base_vertices_);

  // load base mesh
  for (i=0; i<n_base_vertices_; ++i)
  {
    OpenMesh::IO::restore(ifs, p, swap);
    OpenMesh::IO::restore(ifs, radius, swap);
    OpenMesh::IO::restore(ifs, normal, swap);
    OpenMesh::IO::restore(ifs, sin_square, swap);
    OpenMesh::IO::restore(ifs, mue_square, swap);
    OpenMesh::IO::restore(ifs, sigma_square, swap);

    vertex_handle = mesh_.add_vertex(p);
    node_index = vhierarchy_.generate_node_index(i, 1);
    node_handle = vhierarchy_.add_node();

    vhierarchy_.node(node_handle).set_index(node_index);
    vhierarchy_.node(node_handle).set_vertex_handle(vertex_handle);
    mesh_.data(vertex_handle).set_vhierarchy_node_handle(node_handle);
    
    vhierarchy_.node(node_handle).set_radius(radius);
    vhierarchy_.node(node_handle).set_normal(normal);
    vhierarchy_.node(node_handle).set_sin_square(sin_square);
    vhierarchy_.node(node_handle).set_mue_square(mue_square);
    vhierarchy_.node(node_handle).set_sigma_square(sigma_square);
    mesh_.set_normal(vertex_handle, normal);

    index2handle_map[node_index] = node_handle;
    roots.push_back(node_handle);
  }
  vfront_.init(roots, n_details_);

  for (i=0; i<n_base_faces_; ++i)
  {
    OpenMesh::IO::restore(ifs, fvi[0], swap);
    OpenMesh::IO::restore(ifs, fvi[1], swap);
    OpenMesh::IO::restore(ifs, fvi[2], swap);

    mesh_.add_face(mesh_.vertex_handle(fvi[0]), 
      mesh_.vertex_handle(fvi[1]), 
      mesh_.vertex_handle(fvi[2]));
  }

  // load details
  for (i=0; i<n_details_; ++i)
  {
    // position of v0
    OpenMesh::IO::restore(ifs, p, swap);

    // vsplit info.
    OpenMesh::IO::restore(ifs, value, swap);
    node_index = VHierarchyNodeIndex(value);
    
    OpenMesh::IO::restore(ifs, value, swap);
    fund_lcut_index = VHierarchyNodeIndex(value);
    
    OpenMesh::IO::restore(ifs, value, swap);
    fund_rcut_index = VHierarchyNodeIndex(value);

    node_handle = index2handle_map[node_index];
    vhierarchy_.make_children(node_handle);
    lchild_handle = vhierarchy_.node(node_handle).lchild_handle();
    rchild_handle = vhierarchy_.node(node_handle).rchild_handle();

    vhierarchy_.node(node_handle).set_fund_lcut(fund_lcut_index);
    vhierarchy_.node(node_handle).set_fund_rcut(fund_rcut_index);

    vertex_handle = mesh_.add_vertex(p);
    vhierarchy_.node(lchild_handle).set_vertex_handle(vertex_handle);
    vhierarchy_.node(rchild_handle).set_vertex_handle(vhierarchy_.node(node_handle).vertex_handle());    

    index2handle_map[vhierarchy_.node(lchild_handle).node_index()] = lchild_handle;
    index2handle_map[vhierarchy_.node(rchild_handle).node_index()] = rchild_handle;

    // view-dependent parameters
    OpenMesh::IO::restore(ifs, radius, swap);
    OpenMesh::IO::restore(ifs, normal, swap);
    OpenMesh::IO::restore(ifs, sin_square, swap);
    OpenMesh::IO::restore(ifs, mue_square, swap);
    OpenMesh::IO::restore(ifs, sigma_square, swap);

    vhierarchy_.node(lchild_handle).set_radius(radius);
    vhierarchy_.node(lchild_handle).set_normal(normal);
    vhierarchy_.node(lchild_handle).set_sin_square(sin_square);
    vhierarchy_.node(lchild_handle).set_mue_square(mue_square);
    vhierarchy_.node(lchild_handle).set_sigma_square(sigma_square);


    OpenMesh::IO::restore(ifs, radius, swap);
    OpenMesh::IO::restore(ifs, normal, swap);
    OpenMesh::IO::restore(ifs, sin_square, swap);
    OpenMesh::IO::restore(ifs, mue_square, swap);
    OpenMesh::IO::restore(ifs, sigma_square, swap);

    vhierarchy_.node(rchild_handle).set_radius(radius);
    vhierarchy_.node(rchild_handle).set_normal(normal);
    vhierarchy_.node(rchild_handle).set_sin_square(sin_square);
    vhierarchy_.node(rchild_handle).set_mue_square(mue_square);
    vhierarchy_.node(rchild_handle).set_sigma_square(sigma_square);
  }

  ifs.close();

  // update face and vertex normals
  mesh_.update_face_normals();

  // bounding box
  MyMesh::ConstVertexIter  
     vIt(mesh_.vertices_begin()), 
     vEnd(mesh_.vertices_end());

   bbMin_ = bbMax_ = mesh_.point(vIt);
   for (; vIt!=vEnd; ++vIt)
   {
      bbMin_.minimize(mesh_.point(vIt));
      bbMax_.maximize(mesh_.point(vIt));
   }
  
  // set center and radius
  set_scene_pos(0.5f*(bbMin_ + bbMax_), 0.5*(bbMin_ - bbMax_).norm());
    
  // info
  std::cerr << mesh_.n_vertices() << " vertices, "
    << mesh_.n_edges()    << " edge, "
    << mesh_.n_faces()    << " faces, "
    << n_details_ << " detail vertices\n";

  updateGL();
}



void
VDPMClientViewerWidget::
open_vd_base_mesh(const char* _filename)
{
  unsigned int                    i;
  unsigned int                    fvi[3];
  //int                             tree_id, node_id;
  char                            fileformat[16];
  OpenMesh::Vec3f                 p, normal;
  float                           radius, sin_square, mue_square, sigma_square;
  VHierarchyNodeHandleContainer   roots;
  OpenMesh::VertexHandle          vertex_handle;  
  VHierarchyNodeIndex             node_index, lchild_node_index, rchild_node_index, fund_lcut_index, fund_rcut_index;
  VHierarchyNodeHandle            node_handle, lchild_node_handle, rchild_node_handle;  
  std::map<VHierarchyNodeIndex, VHierarchyNodeHandle> index2handle_map;

  std::ifstream ifs(_filename, std::ios::binary);
  if (!ifs)
  {
    std::cerr << "Filename : " << _filename << std::endl;
    std::cerr << "read error\n";
    exit(1);
  }

  //
  bool swap = OpenMesh::Endian::local() != OpenMesh::Endian::LSB;

  // read header  
  ifs.read(fileformat, 10); fileformat[10] = '\0';
  if (std::string(fileformat) != std::string("VDProgMesh"))
  {
    std::cerr << "Wrong file format.\n";
    ifs.close();
    exit(1);
  }
  OpenMesh::IO::restore(ifs, n_base_vertices_, swap);
  OpenMesh::IO::restore(ifs, n_base_faces_, swap);
  OpenMesh::IO::restore(ifs, n_details_, swap);  
    
  mesh_.clear();
  vfront_.clear();
  vhierarchy_.clear();

  vhierarchy_.set_num_roots(n_base_vertices_);

  // load base mesh
  for (i=0; i<n_base_vertices_; ++i)
  {
    OpenMesh::IO::restore(ifs, p, swap);
    OpenMesh::IO::restore(ifs, radius, swap);
    OpenMesh::IO::restore(ifs, normal, swap);
    OpenMesh::IO::restore(ifs, sin_square, swap);
    OpenMesh::IO::restore(ifs, mue_square, swap);
    OpenMesh::IO::restore(ifs, sigma_square, swap);

    vertex_handle = mesh_.add_vertex(p);
    node_index = vhierarchy_.generate_node_index(i, 1);
    node_handle = vhierarchy_.add_node();

    vhierarchy_.node(node_handle).set_index(node_index);
    vhierarchy_.node(node_handle).set_vertex_handle(vertex_handle);
    mesh_.data(vertex_handle).set_vhierarchy_node_handle(node_handle);
    
    vhierarchy_.node(node_handle).set_radius(radius);
    vhierarchy_.node(node_handle).set_normal(normal);
    vhierarchy_.node(node_handle).set_sin_square(sin_square);
    vhierarchy_.node(node_handle).set_mue_square(mue_square);
    vhierarchy_.node(node_handle).set_sigma_square(sigma_square);
    mesh_.set_normal(vertex_handle, normal);

    index2handle_map[node_index] = node_handle;
    roots.push_back(node_handle);

#ifdef EXAMPLE_CREATION
    g_index2numdesc_map[node_index] = 1;
#endif

  }
  vfront_.init(roots, n_details_);

  for (i=0; i<n_base_faces_; ++i)
  {
    OpenMesh::IO::restore(ifs, fvi[0], swap);
    OpenMesh::IO::restore(ifs, fvi[1], swap);
    OpenMesh::IO::restore(ifs, fvi[2], swap);

    mesh_.add_face(mesh_.vertex_handle(fvi[0]), 
      mesh_.vertex_handle(fvi[1]), 
      mesh_.vertex_handle(fvi[2]));
  }



#ifdef EXAMPLE_CREATION

  // load details
  for (i=0; i<n_details_; ++i)
  {
    // position of v0
    OpenMesh::IO::restore(ifs, p, swap);

    // vsplit info.
    OpenMesh::IO::restore(ifs, value, swap);
    node_index = vhierarchy_.generate_node_index(value);    
    
    // fundamental cut vertices
    OpenMesh::IO::restore(ifs, value, swap);
    OpenMesh::IO::restore(ifs, value, swap);
    
    // view-dependent parameters
    OpenMesh::IO::restore(ifs, radius, swap);
    OpenMesh::IO::restore(ifs, normal, swap);
    OpenMesh::IO::restore(ifs, sin_square, swap);
    OpenMesh::IO::restore(ifs, mue_square, swap);
    OpenMesh::IO::restore(ifs, sigma_square, swap);
    OpenMesh::IO::restore(ifs, radius, swap);
    OpenMesh::IO::restore(ifs, normal, swap);
    OpenMesh::IO::restore(ifs, sin_square, swap);
    OpenMesh::IO::restore(ifs, mue_square, swap);
    OpenMesh::IO::restore(ifs, sigma_square, swap);

    __add_children(node_index, false);
  }

  // set max_descendents of root vertices

  for (i=0; i<n_base_vertices_; ++i)
  {
    node_index = vhierarchy_.generate_node_index(i, 1);
    VHierarchyNodeHandle node_handle = index2handle_map[node_index];
    VertexHandle  vh = vhierarchy_.node(node_handle).vertex_handle();

    mesh_.set_color(vh, myYellow);
  }
#endif

  ifs.close();

  // update face and vertex normals
  mesh_.update_face_normals();

  // bounding box
  MyMesh::ConstVertexIter  
     vIt(mesh_.vertices_begin()), 
     vEnd(mesh_.vertices_end());

   bbMin_ = bbMax_ = mesh_.point(vIt);
   for (; vIt!=vEnd; ++vIt)
   {
      bbMin_.minimize(mesh_.point(vIt));
      bbMax_.maximize(mesh_.point(vIt));
   }
  
  // set center and radius
  set_scene_pos(0.5f*(bbMin_ + bbMax_), 0.5*(bbMin_ - bbMax_).norm());

  save_views();
    
  // info
  std::cerr << mesh_.n_vertices() << " vertices, "
    << mesh_.n_edges()    << " edge, "
    << mesh_.n_faces()    << " faces, "
    << n_details_ << " detail vertices\n";

  updateGL();
}



void VDPMClientViewerWidget::keyPressEvent(QKeyEvent* _event)
{
  bool handled(false);

  switch (_event->key())
  {
    case Qt::Key_Plus:
      viewing_parameters_.increase_tolerance();
      std::cout << "Scree-space error tolerance^2 is increased by " 
		<< viewing_parameters_.tolerance_square() << std::endl;
      updateGL();
      break;

    case Qt::Key_Minus:
      viewing_parameters_.decrease_tolerance();
      std::cout << "Screen-space error tolerance^2 is decreased by " 
		<< viewing_parameters_.tolerance_square() << std::endl;
      updateGL();
      break;

    case Qt::Key_A:
      adaptive_mode_ = !(adaptive_mode_);
      std::cout << "Adaptive refinement mode is " 
		<< ((adaptive_mode_ == true) ? "on" : "off") << std::endl;
      updateGL();
      break;

    case Qt::Key_D:
      set_debug_print(!debug_print());
      break;

    case Qt::Key_O:
      qFilename_ = QFileDialog::getOpenFileName(0,"","d:/data/models/spm/", "*.spm");
      open_vd_base_mesh( qFilename_.toStdString().c_str() );
      break;

    case Qt::Key_BracketLeft:
      max_transmitted_datasize_ -= 10000;
      std::cout << "Max transmitted data: " << max_transmitted_datasize_ << std::endl;
      break;

    case Qt::Key_BracketRight:
      max_transmitted_datasize_ += 10000;
      std::cout << "Max transmitted data: " << max_transmitted_datasize_ << std::endl;
      break;

    case Qt::Key_Space:
      memcpy(reserved_modelview_matrix_, modelview_matrix(), 16*sizeof(GLdouble));
      memcpy(reserved_projection_matrix_, projection_matrix(), 16*sizeof(GLdouble));
      std::cout << "Reserving current view-point" << std::endl;
      break;

    case Qt::Key_R:
      request_base_mesh();
      break;

    case Qt::Key_S:
      save_views();
      std::cout << "Saving view-points" << std::endl;
      break;

    case Qt::Key_F:
      frame_ = 0;
      std::cout << "Frame is set to 0" << std::endl;
      break;

    case Qt::Key_M:
      adaptive_mode_ = false;
      current_max_resolution();
      updateGL();
      std::cout << "Current max resolution mesh" << std::endl;
      break;

    case Qt::Key_V:
      vd_streaming_ = !(vd_streaming_);
      if (vd_streaming_)
        std::cout << "View-dependent streaing mode" << std::endl;
      else
        std::cout << "Sequential streaming mode" << std::endl;
      break;

    case Qt::Key_C:
      adaptive_mode_ = false;
      qCameraFileName_ = QFileDialog::getOpenFileName(0, "","./", "*.cmr");
      load_views( qCameraFileName_.toStdString().c_str() );
      std::cout << "Loading view-points" << std::endl;
      updateGL();
      break;

    case Qt::Key_9:
      save_screen(true);
      break;

    case Qt::Key_0:
      std::cout << "#faces: " << mesh_.n_faces() << std::endl;
      break;

    case Qt::Key_P:      
      if (qAnimationTimer_->isActive())
      {
        qAnimationTimer_->stop();
        std::cout << "print_statistics mode is stopped!" << std::endl;
      }
      else
      {
        qAnimationTimer_->setSingleShot( true );
        qAnimationTimer_->start(0);
        std::cout << "print_statistics mode is started!" << std::endl;

        adaptive_mode_ = true;

        set_scene_pos(
          Vec3f(0.5f*(bbMin_[0] + bbMax_[0]), 0.9f*bbMax_[1], 0.5f*(bbMin_[2] + bbMax_[2])),
          0.15f*(bbMin_ - bbMax_).norm());
        nth_viewpoint_ = 0;
        print_statistics();        
      }
      std::cout << "Frame: " << frame_ << std::endl;
      break;


    case Qt::Key_L:
      if (qAnimationTimer_->isActive())
      {
        qAnimationTimer_->stop();
        std::cout << "look_around mode is stopped!" << std::endl;
      }
      else
      {
        qAnimationTimer_->setSingleShot( true );
        qAnimationTimer_->start(0);
        std::cout << "look_around mode is started!" << std::endl;

        adaptive_mode_ = true;

        set_scene_pos(
          Vec3f(0.5f*(bbMin_[0] + bbMax_[0]), 0.9f*bbMax_[1], 0.5f*(bbMin_[2] + bbMax_[2])),
          0.15f*(bbMin_ - bbMax_).norm());                
        frame_ = 0;
        nth_viewpoint_ = 0;
        look_around();
      }

      break;
  
    case Qt::Key_Q:
    case Qt::Key_Escape:
      qApp->quit();

    default:
      this->MeshViewerWidget::keyPressEvent( _event );
  }
  
  if (!handled)
    _event->ignore();
}





void
VDPMClientViewerWidget::
update_viewing_parameters()
{  
  viewing_parameters_.set_fovy(fovy());
  viewing_parameters_.set_aspect((float) width() / (float) height());
  viewing_parameters_.set_modelview_matrix(modelview_matrix());

  viewing_parameters_.update_viewing_configurations();
}


/////////////////////////////////////////////////
// streaming related functions
/////////////////////////////////////////////////



bool
VDPMClientViewerWidget::
request_base_mesh()
{
  if (streaming_phase_ != kBaseMesh)
    return  false;

  if (qFilename_.isEmpty() == true)
  {
    std::cout << "Please, specify the base mesh filename." << std::endl;
    return  false;
  }

  QDataStream qTcp(qSocket_);

  qTcp << qFilename_.length();
  qTcp << qFilename_;
  qSocket_->flush();
  
  return  true;
}

bool
VDPMClientViewerWidget::
receive_base_mesh()
{
  int status;
  QDataStream qTcp(qSocket_);

  while ( qSocket_->bytesAvailable() < sizeof(int) )
    qSocket_->waitForReadyRead(10);

  qTcp >> status;
  if (status == 0)
  {
    std::cout << "There is no such a VDPM files in the server side." << std::endl;
    return  false;
  }

  streaming_phase_ = kVSplitHeader;

  std::cout << "A view-dependent streaming is ready." << std::endl;
  return  true;
}


void
VDPMClientViewerWidget::
send_viewing_information()
{
  session_timer_.start();
  QDataStream qTCP(qSocket_);
  qTCP  << modelview_matrix()[0]  << modelview_matrix()[1]
        << modelview_matrix()[2]  << modelview_matrix()[3]
        << modelview_matrix()[4]  << modelview_matrix()[5]  
	      << modelview_matrix()[6]  << modelview_matrix()[7]
        << modelview_matrix()[8]  << modelview_matrix()[9]
	      << modelview_matrix()[10] << modelview_matrix()[11]
        << modelview_matrix()[12] << modelview_matrix()[13]
	      << modelview_matrix()[14] << modelview_matrix()[15]
        << viewing_parameters_.fovy()
        << viewing_parameters_.aspect()
        << viewing_parameters_.tolerance_square();
  
  qSocket_->flush();
  session_timer_.stop();
  fprintf(session_file, "%d %d\n", frame_, (int) session_timer_.mseconds());
  

  global_timer_.stop();
  fprintf(uplink_file, "%d %ld\n", (int) global_timer_.mseconds(), 16*sizeof(double) + 3*sizeof(float));
  global_timer_.cont();
}

void
VDPMClientViewerWidget::
receive_vsplit_header()
{
  if (qSocket_->bytesAvailable() < sizeof(unsigned int))
    return;

  QDataStream qTcp(qSocket_);

  // while (qSocket_->waitForMore(10) < sizeof(unsigned int));

  qTcp >> n_vsplit_packets_;

  if (n_vsplit_packets_ > 0)
  {
    streaming_phase_ = kVSplits;

    if (debug_print() == true)
    {
      std::cout << "Server will transmit " << n_vsplit_packets_ 
	        << " of vsplit packets to me" << std::endl;
    }

    receive_vsplit_packets();
  }
  else
  {
    session_running_ = false;
  }
}

void
VDPMClientViewerWidget::
receive_vsplit_packets()
{
  static unsigned int n_vsplits = 0;
  static unsigned int          len = (int) (17 * sizeof(float) + 3 * sizeof(int));
  
  if (qSocket_->bytesAvailable() < len)
    return;

  QString             str;
  OpenMesh::Vec3f     pos;
  VHierarchyNodeIndex node_index, fund_lcut_index, fund_rcut_index;
  float               radius[2];
  OpenMesh::Vec3f     normal[2];
  float               sin_square[2];
  float               mue_square[2];
  float               sigma_square[2];
  unsigned int        value[3];

  global_timer_.stop();
  fprintf(downlink_file, "%d %ld\n", (int) global_timer_.mseconds(), qSocket_->bytesAvailable());
  global_timer_.cont();

  session_timer_.start();

  while ( qSocket_->bytesAvailable() >= len )
  {
    if (vd_streaming_)  transmitted_datasize_ += (int) len;
    //if (vd_streaming_)  transmitted_datasize_ += (int) 3*sizeof(int) + 3*sizeof(float); // only for non-refinement cliet
    else                transmitted_datasize_ += (int) 3*sizeof(int) + 3*sizeof(float);

    if (max_transmitted_datasize_ > 0)
    {      
      if (transmitted_datasize_ > max_transmitted_datasize_)
      {
        if (vd_streaming_)  transmitted_datasize_ -= (int) len;
        //if (vd_streaming_)  transmitted_datasize_ -= (int) 3*sizeof(int) + 3*sizeof(float); // only for non-refinement cliet
        else                transmitted_datasize_ -= (int) 3*sizeof(int) + 3*sizeof(float);

        return;
      }        
    }

    QDataStream qTcp(qSocket_);  
    qTcp  >> pos[0] >> pos[1] >> pos[2]
	  >> value[0]
	  >> value[1]
	  >> value[2]
	  >> radius[0]
	  >> (normal[0])[0] >> (normal[0])[1] >> (normal[0])[2]
	  >> sin_square[0]
	  >> mue_square[0]  >> sigma_square[0]
	  >> radius[1]
	  >> (normal[1])[0] >> (normal[1])[1] >> (normal[1])[2]
	  >> sin_square[1]
	  >> mue_square[1]  >> sigma_square[1];

    node_index      = VHierarchyNodeIndex(value[0]);
    fund_lcut_index = VHierarchyNodeIndex(value[1]);
    fund_rcut_index = VHierarchyNodeIndex(value[2]);

    update_vhierarchy(pos, 
		      node_index, fund_lcut_index, fund_rcut_index,
		      radius, normal, sin_square, mue_square, sigma_square);


    std::cout << "transmitted datasize: " << transmitted_datasize_ << std::endl;
 
    if (debug_print() == true)
    {
      std::cout << "Pkg #" << n_vsplits << std::endl; 
    }

    ++n_vsplits;

    if (n_vsplits >= n_vsplit_packets_)
    {
      n_vsplits        = 0;
      streaming_phase_ = kVSplitHeader;
      session_running_ = false;

      if (debug_print() == true)
      {
        std::cout << "transmission of vsplit packets is complete" << std::endl;
      }
      break;
    }
  }

  session_timer_.stop();
  fprintf(session_file, "%d %d\n", frame_, (int) session_timer_.mseconds());

  updateGL();

  if (n_vsplits != n_vsplit_packets_){
    qSessionTimer_->setSingleShot( true );
    qSessionTimer_->start(300);
  }
}


void 
VDPMClientViewerWidget::
update_vhierarchy(
  const OpenMesh::Vec3f     &_pos,              // 3D position of v0
  const VHierarchyNodeIndex &_node_index,       // vhierarchy index of v1
  const VHierarchyNodeIndex &_fund_lcut_index,  // vhierarchy index of fundamental lcut
  const VHierarchyNodeIndex &_fund_rcut_index,  // vhierarchy index of fundamental rcut
  const float               _radius[2],         // radius of lchild & rchild
  const OpenMesh::Vec3f     _normal[2],         // normal of lchild & rchild
  const float               _sin_square[2],     // sin_square of lchild & rchild
  const float               _mue_square[2],     // mue_square of lchild & rchild
  const float               _sigma_square[2]    // sigma_square of lchild & rchild
  )
{
  OpenMesh::VertexHandle  vertex_handle;
  VHierarchyNodeHandle    node_handle, lchild_handle, rchild_handle;
  
  node_handle = vhierarchy_.node_handle(_node_index);
  vhierarchy_.make_children(node_handle);
  

  lchild_handle = vhierarchy_.lchild_handle(node_handle);
  rchild_handle = vhierarchy_.rchild_handle(node_handle);

  vhierarchy_.node(node_handle).set_fund_lcut(_fund_lcut_index);
  vhierarchy_.node(node_handle).set_fund_rcut(_fund_rcut_index);

  vertex_handle = mesh_.add_vertex(_pos);
  vhierarchy_.node(lchild_handle).set_vertex_handle(vertex_handle);
  vhierarchy_.node(rchild_handle).set_vertex_handle(vhierarchy_.node(node_handle).vertex_handle());

  vhierarchy_.node(lchild_handle).set_radius(_radius[0]);
  vhierarchy_.node(lchild_handle).set_normal(_normal[0]);
  vhierarchy_.node(lchild_handle).set_sin_square(_sin_square[0]);
  vhierarchy_.node(lchild_handle).set_mue_square(_mue_square[0]);
  vhierarchy_.node(lchild_handle).set_sigma_square(_sigma_square[0]);

  vhierarchy_.node(rchild_handle).set_radius(_radius[1]);
  vhierarchy_.node(rchild_handle).set_normal(_normal[1]);
  vhierarchy_.node(rchild_handle).set_sin_square(_sin_square[1]);
  vhierarchy_.node(rchild_handle).set_mue_square(_mue_square[1]);
  vhierarchy_.node(rchild_handle).set_sigma_square(_sigma_square[1]);

#ifdef EXAMPLE_CREATION
  __add_children(_node_index);
#endif
}



/////////////////////////////////////////////////
// example related functions
/////////////////////////////////////////////////

void VDPMClientViewerWidget::save_views()
{
  FILE *camera_file = fopen("camera.cmr", "w");

  GLdouble  current_modelview_matrix[16], current_projection_matrix[16];
  
  memcpy(current_modelview_matrix, modelview_matrix(), 16*sizeof(GLdouble));
  memcpy(current_projection_matrix, projection_matrix(), 16*sizeof(GLdouble));


  fprintf(camera_file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
    current_modelview_matrix[0],  current_modelview_matrix[1],  current_modelview_matrix[2],  current_modelview_matrix[3],
    current_modelview_matrix[4],  current_modelview_matrix[5],  current_modelview_matrix[6],  current_modelview_matrix[7],
    current_modelview_matrix[8],  current_modelview_matrix[9],  current_modelview_matrix[10], current_modelview_matrix[11],
    current_modelview_matrix[12], current_modelview_matrix[13], current_modelview_matrix[14], current_modelview_matrix[15]);
  
  fprintf(camera_file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
    current_projection_matrix[0],  current_projection_matrix[1],  current_projection_matrix[2],  current_projection_matrix[3],
    current_projection_matrix[4],  current_projection_matrix[5],  current_projection_matrix[6],  current_projection_matrix[7],
    current_projection_matrix[8],  current_projection_matrix[9],  current_projection_matrix[10], current_projection_matrix[11],
    current_projection_matrix[12], current_projection_matrix[13], current_projection_matrix[14], current_projection_matrix[15]);

  
  fprintf(camera_file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
    reserved_modelview_matrix_[0],  reserved_modelview_matrix_[1],  reserved_modelview_matrix_[2],  reserved_modelview_matrix_[3],
    reserved_modelview_matrix_[4],  reserved_modelview_matrix_[5],  reserved_modelview_matrix_[6],  reserved_modelview_matrix_[7],
    reserved_modelview_matrix_[8],  reserved_modelview_matrix_[9],  reserved_modelview_matrix_[10], reserved_modelview_matrix_[11],
    reserved_modelview_matrix_[12], reserved_modelview_matrix_[13], reserved_modelview_matrix_[14], reserved_modelview_matrix_[15]);
  
  fprintf(camera_file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
    reserved_projection_matrix_[0],  reserved_projection_matrix_[1],  reserved_projection_matrix_[2],  reserved_projection_matrix_[3],
    reserved_projection_matrix_[4],  reserved_projection_matrix_[5],  reserved_projection_matrix_[6],  reserved_projection_matrix_[7],
    reserved_projection_matrix_[8],  reserved_projection_matrix_[9],  reserved_projection_matrix_[10], reserved_projection_matrix_[11],
    reserved_projection_matrix_[12], reserved_projection_matrix_[13], reserved_projection_matrix_[14], reserved_projection_matrix_[15]);
  
  fclose(camera_file);
}


void VDPMClientViewerWidget::load_views(const char *camera_filename)
{
  FILE *camera_file = fopen(camera_filename, "r");

  GLdouble  current_modelview_matrix[16], current_projection_matrix[16];


  fscanf(camera_file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
    &(current_modelview_matrix[0]),  &(current_modelview_matrix[1]),  &(current_modelview_matrix[2]),  &(current_modelview_matrix[3]),
    &(current_modelview_matrix[4]),  &(current_modelview_matrix[5]),  &(current_modelview_matrix[6]),  &(current_modelview_matrix[7]),
    &(current_modelview_matrix[8]),  &(current_modelview_matrix[9]),  &(current_modelview_matrix[10]), &(current_modelview_matrix[11]),
    &(current_modelview_matrix[12]), &(current_modelview_matrix[13]), &(current_modelview_matrix[14]), &(current_modelview_matrix[15]));


  fscanf(camera_file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
    &(current_projection_matrix[0]),  &(current_projection_matrix[1]),  &(current_projection_matrix[2]),  &(current_projection_matrix[3]),
    &(current_projection_matrix[4]),  &(current_projection_matrix[5]),  &(current_projection_matrix[6]),  &(current_projection_matrix[7]),
    &(current_projection_matrix[8]),  &(current_projection_matrix[9]),  &(current_projection_matrix[10]), &(current_projection_matrix[11]),
    &(current_projection_matrix[12]), &(current_projection_matrix[13]), &(current_projection_matrix[14]), &(current_projection_matrix[15]));


  fscanf(camera_file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
    &(reserved_modelview_matrix_[0]),  &(reserved_modelview_matrix_[1]),  &(reserved_modelview_matrix_[2]),  &(reserved_modelview_matrix_[3]),
    &(reserved_modelview_matrix_[4]),  &(reserved_modelview_matrix_[5]),  &(reserved_modelview_matrix_[6]),  &(reserved_modelview_matrix_[7]),
    &(reserved_modelview_matrix_[8]),  &(reserved_modelview_matrix_[9]),  &(reserved_modelview_matrix_[10]), &(reserved_modelview_matrix_[11]),
    &(reserved_modelview_matrix_[12]), &(reserved_modelview_matrix_[13]), &(reserved_modelview_matrix_[14]), &(reserved_modelview_matrix_[15]));


  fscanf(camera_file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
    &(reserved_projection_matrix_[0]),  &(reserved_projection_matrix_[1]),  &(reserved_projection_matrix_[2]),  &(reserved_projection_matrix_[3]),
    &(reserved_projection_matrix_[4]),  &(reserved_projection_matrix_[5]),  &(reserved_projection_matrix_[6]),  &(reserved_projection_matrix_[7]),
    &(reserved_projection_matrix_[8]),  &(reserved_projection_matrix_[9]),  &(reserved_projection_matrix_[10]), &(reserved_projection_matrix_[11]),
    &(reserved_projection_matrix_[12]), &(reserved_projection_matrix_[13]), &(reserved_projection_matrix_[14]), &(reserved_projection_matrix_[15]));

  fclose(camera_file);

  set_modelview_matrix(current_modelview_matrix);
  set_projection_matrix(current_projection_matrix);

  adaptive_mode_ = false;
}

void VDPMClientViewerWidget::print_statistics()
{
  const float   angle   = 360.0/(float)n_viewpoints_;
  Vec3f         axis    = Vec3f(0,1,0);
  Vec3f         delta   = Vec3f(0, 0.7f*(bbMin_[1] - bbMax_[1])/n_viewpoints_, 0);

  rotate(axis, -angle);
  set_scene_pos(center() + delta, 1.0f * radius() );
  
  updateGL();
 
  if (++nth_viewpoint_ < n_viewpoints_){
    qAnimationTimer_->setSingleShot( true );
    qAnimationTimer_->start(500);
  }
}

void VDPMClientViewerWidget::look_around()
{
  const float   angle   = 360.0/(float)n_viewpoints_;
  Vec3f         axis    = Vec3f(0,1,0);
  Vec3f         delta   = Vec3f(0, 0.7f*(bbMin_[1] - bbMax_[1])/n_viewpoints_, 0);

  rotate(axis, -angle);
  set_scene_pos(center() + delta, 1.0f * radius() );
  
  updateGL();

  save_screen(true);
  
  if (++nth_viewpoint_ < n_viewpoints_){
    qAnimationTimer_->setSingleShot( true );
    qAnimationTimer_->start(3000);
  }
}


void VDPMClientViewerWidget::save_screen(bool _flag)
{
  setCursor( Qt::WaitCursor );

  if (_flag == true)     // shot from the reserved view-point
  {
    GLdouble  current_modelview_matrix[16];
    GLdouble  current_projection_matrix[16];
    bool      current_adaptive_mode = adaptive_mode_;

    memcpy(current_modelview_matrix, modelview_matrix(), 16*sizeof(GLdouble));
    memcpy(current_projection_matrix, projection_matrix(), 16*sizeof(GLdouble));

    set_modelview_matrix(reserved_modelview_matrix_);
    set_projection_matrix(reserved_projection_matrix_);
    adaptive_mode_ = false;

    updateGL();
    
    // shot from the reserved view-point
    char rfilename[256];
    sprintf(rfilename, "rview%03d.bmp", nth_viewpoint_);
    screen_capture(rfilename);
    std::cout << "shot from the reserved view-point" << std::endl;
    
    
    set_modelview_matrix(current_modelview_matrix);
    set_projection_matrix(current_projection_matrix);
    adaptive_mode_ = current_adaptive_mode;
  }

  updateGL();

  // shot from the current view-point
  char cfilename[256];
  sprintf(cfilename, "cview%03d.bmp", nth_viewpoint_);
  screen_capture(cfilename);
  std::cout << "shot from the current view-point" << std::endl;
  

  setCursor( Qt::PointingHandCursor );
}

void VDPMClientViewerWidget::screen_capture(const char * /* _filename */)
{
//   CImageData  image(width(), height());
  
//   glReadBuffer(GL_BACK);
//   glReadPixels(0, 0, width(), height(), GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) image.rgbMap());

//   image.SaveBMP(_filename, width(), height());
}


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



//=============================================================================
//
//  CLASS newClass - IMPLEMENTATION
//
//=============================================================================

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

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/Utils/Endian.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Apps/VDProgMesh/Synthesizer/VDPMSynthesizerViewerWidget.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {


//== IMPLEMENTATION ========================================================== 

VDPMSynthesizerViewerWidget::VDPMSynthesizerViewerWidget(QWidget* _parent, const char* _name)
  : MeshViewerWidget(_parent),
    kappa_square_(0.0),
    adaptive_mode_(false),
    n_base_vertices_(0),
    n_base_edges_(0),
    n_base_faces_(0),
    n_details_(0)

{
  adaptive_mode_ = true;
}

VDPMSynthesizerViewerWidget::~VDPMSynthesizerViewerWidget()
{

}

void
VDPMSynthesizerViewerWidget::
draw_scene(const std::string &_draw_mode)
{
  if (adaptive_mode_ == true)
  {    
    adaptive_refinement();
  }
  MeshViewerWidget::draw_scene(_draw_mode);  
}


void
VDPMSynthesizerViewerWidget::
adaptive_refinement()
{
  update_viewing_parameters();

  VDPMMesh::HalfedgeHandle v0v1;

  float fovy = viewing_parameters_.fovy();

  float tolerance_square = viewing_parameters_.tolerance_square();
  float	tan_value = tanf(fovy / 2.0f);
	
  kappa_square_ = 4.0f * tan_value * tan_value * tolerance_square;

  //assert( !vfront_.end() );

  for ( vfront_.begin(); !vfront_.end(); )
  {
    VHierarchyNodeHandle  
      node_handle   = vfront_.node_handle(),
      parent_handle = vhierarchy_.parent_handle(node_handle);
    
    if (vhierarchy_.is_leaf_node(node_handle) != true && 
        qrefine(node_handle) == true)
    {
      force_vsplit(node_handle);      
    }
    else if (vhierarchy_.is_root_node(node_handle) != true && 
	     ecol_legal(parent_handle, v0v1) == true       && 
	     qrefine(parent_handle) != true)
    {
      ecol(parent_handle, v0v1);
    }
    else
    {
      vfront_.next();
    }
  }

  // free memories tagged as 'deleted'
  mesh_.garbage_collection(false, true, true); 
  mesh_.update_face_normals();
}


bool
VDPMSynthesizerViewerWidget::
qrefine(VHierarchyNodeHandle _node_handle)
{
  VHierarchyNode &node    = vhierarchy_.node(_node_handle);
  Vec3f p       = mesh_.point(node.vertex_handle());  
  Vec3f eye_dir = p - viewing_parameters_.eye_pos();;

  float	distance = eye_dir.length();
  float	distance2 = distance * distance;
  float	product_value = dot(eye_dir, node.normal());

  if (outside_view_frustum(p, node.radius()) == true)
    return	false;

  if (oriented_away(node.sin_square(), distance2, product_value) == true)
    return	false;

  if (screen_space_error(node.mue_square(), 
			 node.sigma_square(), 
			 distance2, 
			 product_value) == true)
    return false;
  
  return true;
}


void 
VDPMSynthesizerViewerWidget::
force_vsplit(VHierarchyNodeHandle node_handle)
{
  VDPMMesh::VertexHandle  vl, vr;

  get_active_cuts(node_handle, vl, vr);

  while (vl == vr) 
  {
    force_vsplit(mesh_.data(vl).vhierarchy_node_handle());
    get_active_cuts(node_handle, vl, vr);
  }
	
  vsplit(node_handle, vl, vr);
}



void
VDPMSynthesizerViewerWidget::
vsplit(VHierarchyNodeHandle _node_handle, 
       VDPMMesh::VertexHandle vl, 
       VDPMMesh::VertexHandle vr)
{
  // refine
  VHierarchyNodeHandle  
    lchild_handle = vhierarchy_.lchild_handle(_node_handle),
    rchild_handle = vhierarchy_.rchild_handle(_node_handle);

  VDPMMesh::VertexHandle  v0 = vhierarchy_.vertex_handle(lchild_handle);
  VDPMMesh::VertexHandle  v1 = vhierarchy_.vertex_handle(rchild_handle);
  
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
VDPMSynthesizerViewerWidget::
ecol(VHierarchyNodeHandle _node_handle, const VDPMMesh::HalfedgeHandle& v0v1)
{
  VHierarchyNodeHandle  
    lchild_handle = vhierarchy_.lchild_handle(_node_handle),
    rchild_handle = vhierarchy_.rchild_handle(_node_handle);

  VDPMMesh::VertexHandle  v0 = vhierarchy_.vertex_handle(lchild_handle);
  VDPMMesh::VertexHandle  v1 = vhierarchy_.vertex_handle(rchild_handle);

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
VDPMSynthesizerViewerWidget::
ecol_legal(VHierarchyNodeHandle _parent_handle, VDPMMesh::HalfedgeHandle& v0v1)
{
  VHierarchyNodeHandle
    lchild_handle = vhierarchy_.lchild_handle(_parent_handle),
    rchild_handle = vhierarchy_.rchild_handle(_parent_handle);

  // test whether lchild & rchild present in the current vfront
  if ( vfront_.is_active(lchild_handle) != true || 
       vfront_.is_active(rchild_handle) != true)
    return  false;  

  VDPMMesh::VertexHandle v0, v1;

  v0 = vhierarchy_.vertex_handle(lchild_handle);
  v1 = vhierarchy_.vertex_handle(rchild_handle);

  v0v1 = mesh_.find_halfedge(v0, v1);
  
  return  mesh_.is_collapse_ok(v0v1);
}

void
VDPMSynthesizerViewerWidget::
get_active_cuts(const VHierarchyNodeHandle _node_handle, 
		VDPMMesh::VertexHandle &vl, VDPMMesh::VertexHandle &vr)
{
  VDPMMesh::VertexVertexIter  vv_it;
  VHierarchyNodeHandle        nnode_handle;

  VHierarchyNodeIndex
    nnode_index, 
    fund_lcut_index = vhierarchy_.fund_lcut_index(_node_handle),
    fund_rcut_index = vhierarchy_.fund_rcut_index(_node_handle);

  vl = VDPMMesh::InvalidVertexHandle;
  vr = VDPMMesh::InvalidVertexHandle;

  for (vv_it=mesh_.vv_iter(vhierarchy_.vertex_handle(_node_handle)); 
      vv_it.is_valid(); ++vv_it)
  {
    nnode_handle = mesh_.data(*vv_it).vhierarchy_node_handle();
    nnode_index = vhierarchy_.node_index(nnode_handle);

    if (vl == VDPMMesh::InvalidVertexHandle && 
        vhierarchy_.is_ancestor(nnode_index, fund_lcut_index) == true)
      vl = *vv_it;

    if (vr == VDPMMesh::InvalidVertexHandle && 
        vhierarchy_.is_ancestor(nnode_index, fund_rcut_index) == true)
      vr = *vv_it;

    /*if (vl == VDPMMesh::InvalidVertexHandle && nnode_index.is_ancestor_index(fund_lcut_index) == true)
      vl = *vv_it;
    if (vr == VDPMMesh::InvalidVertexHandle && nnode_index.is_ancestor_index(fund_rcut_index) == true)
      vr = *vv_it;*/

    if (vl != VDPMMesh::InvalidVertexHandle && 
        vr != VDPMMesh::InvalidVertexHandle)
      break;
  }
}


bool 
VDPMSynthesizerViewerWidget::
outside_view_frustum(const Vec3f &pos, float radius)
{  
#if 0
  return 
    (frustum_plane_[0].signed_distance(pos) < -radius) ||
    (frustum_plane_[1].signed_distance(pos) < -radius) ||
    (frustum_plane_[2].signed_distance(pos) < -radius) ||
    (frustum_plane_[3].signed_distance(pos) < -radius);
#else
  
  Plane3d   frustum_plane[4];
  
  viewing_parameters_.frustum_planes(frustum_plane);

  for (int i = 0; i < 4; i++) {
    if (frustum_plane[i].singed_distance(pos) < -radius)
      return true;
  }  
  return false;
#endif
}

bool 
VDPMSynthesizerViewerWidget::
oriented_away(float sin_square, float distance_square, float product_value)
{
#if 0
  return (product_value > 0)
    &&   ((product_value * product_value) > (distance_square * sin_square));
#else
  if (product_value > 0 && 
      product_value * product_value > distance_square * sin_square)
    return true;
  else
    return false;
#endif
}


bool 
VDPMSynthesizerViewerWidget::
screen_space_error(float mue_square, float sigma_square, 
		   float distance_square, float product_value)
{
#if 0
  float ks_ds = kappa_square_ * distance_square;
  float pv_pv = product_value * product_value;
  return (mue_square >= ks_ds) 
    ||   (sigma_square*( distance_square - pv_pv) >= ks_ds*distance_square);
#else
  if ((mue_square >= kappa_square_ * distance_square) || 
      (sigma_square * (distance_square - product_value * product_value) >= kappa_square_ * distance_square * distance_square))
    return	false;
  else
    return	true;
#endif
}

void
VDPMSynthesizerViewerWidget::
open_vd_prog_mesh(const char* _filename)
{
  unsigned int                    i;
  unsigned int                    value;
  unsigned int                    fvi[3];
  char                            fileformat[16];
  Vec3f                           p, normal;
  float                           radius, sin_square, mue_square, sigma_square;
  VHierarchyNodeHandleContainer   roots;
  VertexHandle                    vertex_handle;  
  VHierarchyNodeIndex             node_index;
  VHierarchyNodeIndex             lchild_node_index, rchild_node_index;
  VHierarchyNodeIndex             fund_lcut_index, fund_rcut_index;
  VHierarchyNodeHandle            node_handle;
  VHierarchyNodeHandle            lchild_node_handle, rchild_node_handle;  

  std::map<VHierarchyNodeIndex, VHierarchyNodeHandle> index2handle_map;

  std::ifstream ifs(_filename, std::ios::binary);

  if (!ifs)
  {
    std::cerr << "read error\n";
    exit(1);
  }

  //
  bool swap = Endian::local() != Endian::LSB;

  // read header  
  ifs.read(fileformat, 10); fileformat[10] = '\0';
  if (std::string(fileformat) != std::string("VDProgMesh"))
  {
    std::cerr << "Wrong file format.\n";
    ifs.close();
    exit(1);
  }

  IO::restore(ifs, n_base_vertices_, swap);
  IO::restore(ifs, n_base_faces_, swap);
  IO::restore(ifs, n_details_, swap);
  
  mesh_.clear();
  vfront_.clear();
  vhierarchy_.clear();

  vhierarchy_.set_num_roots(n_base_vertices_);

  // load base mesh
  for (i=0; i<n_base_vertices_; ++i)
  {
    IO::restore(ifs, p, swap);
    IO::restore(ifs, radius, swap);
    IO::restore(ifs, normal, swap);
    IO::restore(ifs, sin_square, swap);
    IO::restore(ifs, mue_square, swap);
    IO::restore(ifs, sigma_square, swap);

    vertex_handle = mesh_.add_vertex(p);
    node_index    = vhierarchy_.generate_node_index(i, 1);
    node_handle   = vhierarchy_.add_node();

    VHierarchyNode &node = vhierarchy_.node(node_handle);

    node.set_index(node_index);
    node.set_vertex_handle(vertex_handle);
    mesh_.data(vertex_handle).set_vhierarchy_node_handle(node_handle);
    
    node.set_radius(radius);
    node.set_normal(normal);
    node.set_sin_square(sin_square);
    node.set_mue_square(mue_square);
    node.set_sigma_square(sigma_square);
    mesh_.set_normal(vertex_handle, normal);

    index2handle_map[node_index] = node_handle;
    roots.push_back(node_handle);
  }
  vfront_.init(roots, n_details_);

  for (i=0; i<n_base_faces_; ++i)
  {
    IO::restore(ifs, fvi[0], swap);
    IO::restore(ifs, fvi[1], swap);
    IO::restore(ifs, fvi[2], swap);

    mesh_.add_face(mesh_.vertex_handle(fvi[0]),
		   mesh_.vertex_handle(fvi[1]),
		   mesh_.vertex_handle(fvi[2]));
  }

  // load details
  for (i=0; i<n_details_; ++i)
  {
    // position of v0
    IO::restore(ifs, p, swap);
    
    // vsplit info.
    IO::restore(ifs, value, swap);
    node_index = VHierarchyNodeIndex(value);

    IO::restore(ifs, value, swap);
    fund_lcut_index = VHierarchyNodeIndex(value);

    IO::restore(ifs, value, swap);
    fund_rcut_index = VHierarchyNodeIndex(value);


    node_handle = index2handle_map[node_index];
    vhierarchy_.make_children(node_handle);

    VHierarchyNode &node   = vhierarchy_.node(node_handle);
    VHierarchyNode &lchild = vhierarchy_.node(node.lchild_handle());
    VHierarchyNode &rchild = vhierarchy_.node(node.rchild_handle());

    node.set_fund_lcut(fund_lcut_index);
    node.set_fund_rcut(fund_rcut_index);
    
    vertex_handle = mesh_.add_vertex(p);
    lchild.set_vertex_handle(vertex_handle);
    rchild.set_vertex_handle(node.vertex_handle());    

    index2handle_map[lchild.node_index()] = node.lchild_handle();
    index2handle_map[rchild.node_index()] = node.rchild_handle();

    // view-dependent parameters
    IO::restore(ifs, radius, swap);
    IO::restore(ifs, normal, swap);
    IO::restore(ifs, sin_square, swap);
    IO::restore(ifs, mue_square, swap);
    IO::restore(ifs, sigma_square, swap);
    lchild.set_radius(radius);
    lchild.set_normal(normal);
    lchild.set_sin_square(sin_square);
    lchild.set_mue_square(mue_square);
    lchild.set_sigma_square(sigma_square);

    IO::restore(ifs, radius, swap);
    IO::restore(ifs, normal, swap);
    IO::restore(ifs, sin_square, swap);
    IO::restore(ifs, mue_square, swap);
    IO::restore(ifs, sigma_square, swap);
    rchild.set_radius(radius);
    rchild.set_normal(normal);
    rchild.set_sin_square(sin_square);
    rchild.set_mue_square(mue_square);
    rchild.set_sigma_square(sigma_square);
  }

  ifs.close();

  // update face and vertex normals
  mesh_.update_face_normals();

  // bounding box
  VDPMMesh::ConstVertexIter  
     vIt(mesh_.vertices_begin()), 
     vEnd(mesh_.vertices_end());

   VDPMMesh::Point bbMin, bbMax;

   bbMin = bbMax = mesh_.point(*vIt);
   for (; vIt!=vEnd; ++vIt)
   {
      bbMin.minimize(mesh_.point(*vIt));
      bbMax.maximize(mesh_.point(*vIt));
   }
  
  // set center and radius
  set_scene_pos(0.5f*(bbMin + bbMax), 0.5*(bbMin - bbMax).norm());
    
  // info
  std::cerr << mesh_.n_vertices() << " vertices, "
    << mesh_.n_edges()    << " edge, "
    << mesh_.n_faces()    << " faces, "
    << n_details_ << " detail vertices\n";

  updateGL();
}


void VDPMSynthesizerViewerWidget::keyPressEvent(QKeyEvent* _event)
{
  switch (_event->key())
  {
    case Key_Home:
      updateGL();
      break;
    
    case Key_End:
      updateGL();
      break;
      
    case Key_Minus:
      viewing_parameters_.increase_tolerance();
      std::cout << "Scree-space error tolerance^2 is increased by " 
		<< viewing_parameters_.tolerance_square() << std::endl;
      updateGL();
      break;
    
    case Key_Plus:
      viewing_parameters_.decrease_tolerance();
      std::cout << "Screen-space error tolerance^2 is decreased by " 
		<< viewing_parameters_.tolerance_square() << std::endl;
      updateGL();
      break;
      
    case Key_A:
      adaptive_mode_ = !(adaptive_mode_);
      std::cout << "Adaptive refinement mode is " 
		<< (adaptive_mode_ ? "on" : "off") << std::endl;
      updateGL();
      break;

    case Key_O:
      qFilename_ = QFileDialog::getOpenFileName(0,"", "", "*.spm");
      open_vd_prog_mesh( qFilename_.toStdString().c_str() );
      break;
      
    default:
      MeshViewerWidget::keyPressEvent( _event );
  }
  
}





void
VDPMSynthesizerViewerWidget::
update_viewing_parameters()
{
  viewing_parameters_.set_modelview_matrix(modelview_matrix());
  viewing_parameters_.set_aspect((float) width()/ (float) height());
  viewing_parameters_.set_fovy(fovy());
  
  viewing_parameters_.update_viewing_configurations();
}


//=============================================================================
} // namespace OpenMesh
//=============================================================================

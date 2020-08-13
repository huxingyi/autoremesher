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

#include <OpenMesh/Apps/VDProgMesh/Streaming/Server/VDPMServerSession.hh>
#include <OpenMesh/Apps/VDProgMesh/Streaming/Server/VDPMServerViewerWidget.hh>
#include <QTextStream>
#include <QByteArray>


using OpenMesh::VDPM::VHierarchyNode;
using OpenMesh::VDPM::Plane3d;
using OpenMesh::VDPM::InvalidVHierarchyNodeHandle;
using OpenMesh::VDPM::debug_print;


bool
VDPMServerSession::
set_vdpm(const char _vdpm_name[256])
{
#ifdef DEBUG_COUT
  if (debug_print() == true)
  {
    std::cout << "Setting vdpm" << std::endl;
  }
#endif
  vdpm_ = ((VDPMServerViewerWidget *) ((VDPMServerSocket *) parent())->parent())->get_vdpm(_vdpm_name);
  if (vdpm_ == NULL)
    return  false;

  vhierarchy_ = &vdpm_->vhierarchy();

  VHierarchyNodeHandleContainer roots;
  unsigned int  n_details;

  vdpm_->vhierarchy_roots(roots);
  n_details = vdpm_->n_details();

  std::cout << "Details : " << n_details << std::endl;
  //vfront_.init(roots, n_details);

  // vertex hierarchy window
  vhwindow_.set_vertex_hierarchy(vdpm_->vhierarchy());
  vhwindow_.init(roots);


#ifdef DEBUG_COUT
  if (debug_print() == true)
  {
    std::cout << "setting vdpm ended" << std::endl;
  }
#endif  

  return  true;
}


void
VDPMServerSession::
sendBaseMeshToClient()
{
#ifdef DEBUG_COUT
  if (debug_print() == true)
  {
    std::cout << "starting sendBaseMeshToClient()" << std::endl;
  }
#endif



#ifdef DEBUG_COUT
  if (debug_print() == true)
  {
    std::cout << "sendBaseMeshToClient() is done" << std::endl;
  }
#endif
}

void
VDPMServerSession::
send_vsplit_packets()
{
  viewing_parameters_.update_viewing_configurations();
  vsplits_.clear();

  if (((VDPMServerViewerWidget *) this->parent()->parent())->vd_streaming())
    adaptive_refinement();
  else
    sequential_refinement();

  
  if (debug_print() == true)
  {
    std::cout << "adaptive refinement is done on the server side" << std::endl;
  }

  stream_vsplits();

  if (debug_print() == true)
  {
    std::cout << "streameing vsplits is done" << std::endl;
  }
  
  if (transmission_complete_ == true)
  {
    std::cout << "transmission is completed" << std::endl;
  }
  
  if (debug_print() == true)
  {
    std::cout << "send_vsplit_packets() is done" << std::endl;
  }
}


void
VDPMServerSession::
readBaseMeshRequestFromClient()
{
  int           status;
  unsigned int  string_length;
  QDataStream   qTcp(socket_);
  QString       vdpm_name;

//   while (waitForMore(10) < sizeof(int));
  while ( socket_->bytesAvailable() < sizeof(int) )
    socket_->waitForReadyRead(10);

  qTcp >> string_length;

//   while (waitForMore(10) < (string_length*sizeof(char)));
  while (socket_->bytesAvailable() < string_length*sizeof(char) )
    socket_->waitForReadyRead(10);

  qTcp >> vdpm_name;

  if (set_vdpm( vdpm_name.toStdString().c_str() ) != true)  status = 0;
  else                              status = 1;

  qTcp << status;
  socket_->flush();

  if (status == 1)
    streaming_phase_ = kVSplits;
}


void
VDPMServerSession::
readViewingParametersFromClient()
{
  double    modelview_matrix[16];
  float     fovy;
  float     aspect;
  float     tolerance_square;

//   while (waitForMore(10) < 16*sizeof(double) + 3*sizeof(float));
  while ( socket_->bytesAvailable() < 16*sizeof(double) + 3*sizeof(float) )
    socket_->waitForReadyRead(10);

  QDataStream qTCP(socket_);
  qTCP  >> modelview_matrix[0]
        >> modelview_matrix[1]
        >> modelview_matrix[2]
        >> modelview_matrix[3]
        >> modelview_matrix[4] 
        >> modelview_matrix[5] 
        >> modelview_matrix[6]
        >> modelview_matrix[7] 
        >> modelview_matrix[8] 
        >> modelview_matrix[9]
        >> modelview_matrix[10] 
        >> modelview_matrix[11] 
        >> modelview_matrix[12] 
        >> modelview_matrix[13]
        >> modelview_matrix[14] 
        >> modelview_matrix[15]
        >> fovy
        >> aspect
        >> tolerance_square;

  viewing_parameters_.set_modelview_matrix(modelview_matrix);
  viewing_parameters_.set_fovy(fovy);
  viewing_parameters_.set_aspect(aspect);
  viewing_parameters_.set_tolerance_square(tolerance_square);

  send_vsplit_packets();

  fprintf(mem_file, "%d %d %d\n", 
            memory_requirements_using_window(true),
            memory_requirements_using_window(false),
            memory_requirements_using_vfront());
}


void
VDPMServerSession::
PrintOutVFront()
{
}

void
VDPMServerSession::
stream_vsplits()
{
  // send header (i.e., # of vsplits)
  unsigned int          i;
  VHierarchyNodeHandle  node_handle;
  OpenMesh::Vec3f       pos;
  VHierarchyNodeIndex   node_index, fund_lcut_index, fund_rcut_index;
  float                 lchild_radius, rchild_radius;
  OpenMesh::Vec3f       lchild_normal, rchild_normal;
  float                 lchild_sin_square, rchild_sin_square;
  float                 lchild_mue_square, rchild_mue_square;
  float                 lchild_sigma_square, rchild_sigma_square;
  unsigned int          n_vsplit_packets = (unsigned int) vsplits_.size();
  
  QDataStream qTcp(socket_);
  qTcp << n_vsplit_packets;
  socket_->flush();
  
  for (i=0; i<n_vsplit_packets; ++i)
  {
    node_handle = vsplits_[i];
    VHierarchyNodeHandle  lchild_handle = vhierarchy_->lchild_handle(node_handle);
    VHierarchyNodeHandle  rchild_handle = vhierarchy_->rchild_handle(node_handle);
    
    VHierarchyNode &node   = vhierarchy_->node(node_handle);
    VHierarchyNode &lchild = vhierarchy_->node(lchild_handle);
    VHierarchyNode &rchild = vhierarchy_->node(rchild_handle);
    
    pos = vdpm_->point(lchild.vertex_handle());
    node_index = node.node_index();
    fund_lcut_index = node.fund_lcut_index();
    fund_rcut_index = node.fund_rcut_index();
    lchild_radius = lchild.radius();                rchild_radius = rchild.radius();
    lchild_normal = lchild.normal();                rchild_normal = rchild.normal();
    lchild_sin_square = lchild.sin_square();        rchild_sin_square = rchild.sin_square();
    lchild_mue_square = lchild.mue_square();        rchild_mue_square = rchild.mue_square();
    lchild_sigma_square = lchild.sigma_square();    rchild_sigma_square = rchild.sigma_square();

    qTcp  << pos[0] << pos[1] << pos[2]
          << node_index.value()
          << fund_lcut_index.value()
          << fund_rcut_index.value()
          << lchild_radius
          << lchild_normal[0] << lchild_normal[1] << lchild_normal[2]
          << lchild_sin_square
          << lchild_mue_square
          << lchild_sigma_square
          << rchild_radius
          << rchild_normal[0] << rchild_normal[1] << rchild_normal[2]
          << rchild_sin_square
          << rchild_mue_square
          << rchild_sigma_square;
    socket_->flush();    // socket flush
  
    if (debug_print() == true)
    {
      std::cout << "Write to client " << i << "-th vsplit packets: " << std::endl;
    }
  
  }
}


void
VDPMServerSession::
adaptive_refinement()
{
  float fovy = viewing_parameters_.fovy();
  float tolerance_square = viewing_parameters_.tolerance_square();
	float	tan_value = tanf(fovy / 2.0f);
	kappa_square_ = 4.0f * tan_value * tan_value * tolerance_square;

  transmission_complete_ = true;
  for (vhwindow_.begin(); vhwindow_.end() != true; vhwindow_.next())
  {
    VHierarchyNodeHandle  node_handle = vhwindow_.node_handle();

    if (vhierarchy_->is_leaf_node(node_handle) != true)
    {
      transmission_complete_ = false;
      if (qrefine(node_handle) == true)
      {
        force_vsplit(node_handle);
      }
    }
  }
}

void
VDPMServerSession::
sequential_refinement()
{
  for (vhwindow_.begin(); vhwindow_.end() != true; vhwindow_.next())
  {
    VHierarchyNodeHandle  node_handle = vhwindow_.node_handle();
    
    if (vhierarchy_->is_leaf_node(node_handle) != true)
      force_vsplit(node_handle);

    if (vsplits_.size() > 10)
      break;
  }
}


bool
VDPMServerSession::
qrefine(VHierarchyNodeHandle _node_handle)
{
  VHierarchyNode &node = vhierarchy_->node(_node_handle);
  OpenMesh::Vec3f p = vdpm_->point(node.vertex_handle());
  OpenMesh::Vec3f eye_dir = p - viewing_parameters_.eye_pos();
	float	distance = eye_dir.length();
	float	distance2 = distance * distance;
	float	product_value = dot(eye_dir, node.normal());

	if (outside_view_frustum(p, node.radius()) == true)
		return	false;
	if (oriented_away(node.sin_square(), distance2, product_value) == true)
		return	false;
	if (screen_space_error(node.mue_square(), node.sigma_square(), distance2, product_value) == true)
		return	false;

	return	true;
}


bool 
VDPMServerSession::
outside_view_frustum(const OpenMesh::Vec3f &pos, float radius)
{
  Plane3d frustum_plane[4];
  viewing_parameters_.frustum_planes(frustum_plane);

	for (int i = 0; i < 4; i++) {
		if (frustum_plane[i].signed_distance(pos) < -radius)
			return	true;
	}
	
	return false;
}

bool 
VDPMServerSession::
oriented_away(float sin_square, float distance_square, float product_value)
{
	if (product_value > 0 && product_value*product_value > distance_square * sin_square)
		return	true;
	else
		return	false;
}

bool 
VDPMServerSession::
screen_space_error(float mue_square, float sigma_square, float distance_square, float product_value)
{
	if ((mue_square >= kappa_square_ * distance_square) || 
		(sigma_square * (distance_square - product_value * product_value) >= kappa_square_ * distance_square * distance_square))
		return	false;
	else
		return	true;
}


void 
VDPMServerSession::
force_vsplit(VHierarchyNodeHandle node_handle)
{
  VHierarchyNodeHandle  lcut_handle, rcut_handle;
  VHierarchyNodeIndex   fund_lcut_index, fund_rcut_index;

  fund_lcut_index = vhierarchy_->fund_lcut_index(node_handle);
  fund_rcut_index = vhierarchy_->fund_rcut_index(node_handle);

  lcut_handle = active_ancestor_handle(fund_lcut_index);
  rcut_handle = active_ancestor_handle(fund_rcut_index); 
  assert(lcut_handle != InvalidVHierarchyNodeHandle && rcut_handle != InvalidVHierarchyNodeHandle);

	while (lcut_handle == rcut_handle)
  {
    force_vsplit(lcut_handle);
    lcut_handle = active_ancestor_handle(fund_lcut_index);
    rcut_handle = active_ancestor_handle(fund_rcut_index); 
    assert(lcut_handle != InvalidVHierarchyNodeHandle && rcut_handle != InvalidVHierarchyNodeHandle);		
	}
	
	vsplit(node_handle);
}

void
VDPMServerSession::
vsplit(VHierarchyNodeHandle _node_handle)
{  
  // refine
  VHierarchyNodeHandle  lchild_handle = vhierarchy_->lchild_handle(_node_handle);
  VHierarchyNodeHandle  rchild_handle = vhierarchy_->rchild_handle(_node_handle);

  vhwindow_.inactivate(_node_handle);
  vhwindow_.activate(lchild_handle);
  vhwindow_.activate(rchild_handle);

  //vfront_.remove(_node_handle);
  //vfront_.add(lchild_handle);
  //vfront_.add(rchild_handle);

  vsplits_.push_back(_node_handle);
}

VHierarchyNodeHandle 
VDPMServerSession::
active_ancestor_handle(VHierarchyNodeIndex &node_index)
{
  if (node_index.is_valid(vhierarchy_->tree_id_bits()) != true)
    return  InvalidVHierarchyNodeHandle;

  VHierarchyNodeHandle node_handle = vhierarchy_->node_handle(node_index);

  while (node_handle != InvalidVHierarchyNodeHandle && vhwindow_.is_active(node_handle) != true)
    node_handle = vhierarchy_->parent_handle(node_handle);
  
  return  node_handle;
}

unsigned int
VDPMServerSession::
memory_requirements_using_window(bool _estimate)
{
  unsigned int mem = 0;

  // common
  mem += sizeof(VHierarchy*);
  mem += sizeof(ViewingParameters);
  mem += sizeof(float);
  
  if (_estimate)
  {
    unsigned int min = vhierarchy_->num_nodes();
    unsigned int max = 0;

    for (unsigned int i = 0; i < vhierarchy_->num_nodes(); ++i)
    {
      if (vhwindow_.is_active(VHierarchyNodeHandle((int) i)))
      {
        min = std::min(min, i);
        max = std::max(max, i);
      }
    }

    mem += (max - min) / 8;
  }
  else
  {
    mem += vhwindow_.buffer_size();
  }

  return  mem;
}

unsigned int
VDPMServerSession::
memory_requirements_using_vfront()
{
  unsigned int mem = 0;
  std::list<int>  dummy_vfront;
  
  mem += (unsigned int) ceil (vhierarchy_->num_nodes() / 8.0);
  mem += sizeof(dummy_vfront);

  for (unsigned int i = 0; i < vhierarchy_->num_nodes(); ++i)
  {
    if (vhwindow_.is_active(VHierarchyNodeHandle((int) i)))
      mem += 3*sizeof(int);
  }

  return  mem;
}


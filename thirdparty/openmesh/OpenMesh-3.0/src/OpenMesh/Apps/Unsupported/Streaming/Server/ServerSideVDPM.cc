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
#include <fstream>
#include <map>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/Utils/Endian.hh>
#include "ServerSideVDPM.hh"


using OpenMesh::VDPM::VHierarchyNode;
using OpenMesh::VDPM::VHierarchyNodeIndex;
using OpenMesh::VDPM::VHierarchyNodeHandle;


void
ServerSideVDPM::
clear()
{
  points_.clear();
  triangles_.clear();
  vhierarchy_.clear();

  n_base_vertices_ = 0;
  n_base_faces_ = 0;
  n_details_ = 0;
}

OpenMesh::VertexHandle 
ServerSideVDPM::
add_vertex(const OpenMesh::Vec3f &p)
{
  points_.push_back(p);

  return  OpenMesh::VertexHandle(points_.size() - 1);
}

OpenMesh::FaceHandle
ServerSideVDPM::
add_face(const unsigned int _triangle[3])
{
  OpenMesh::Vec3ui  fvi;

  fvi[0] = _triangle[0];
  fvi[1] = _triangle[1];
  fvi[2] = _triangle[2];

  triangles_.push_back(fvi);

  return  OpenMesh::FaceHandle(triangles_.size() - 1);
}


void
ServerSideVDPM::
vhierarchy_roots(VHierarchyNodeHandleContainer &roots) const
{
  unsigned int i;

  roots.clear();

  for (i=0; i<n_base_vertices_; ++i)
  {
    roots.push_back(VHierarchyNodeHandle(i));
  }
}

bool
ServerSideVDPM::
open_vd_prog_mesh(const char *_filename)
{
  unsigned int            i;
  unsigned int            value;
  unsigned int            fvi[3];
  char                    fileformat[16];
  OpenMesh::Vec3f         p, normal;
  float                   radius, sin_square, mue_square, sigma_square;
  OpenMesh::VertexHandle  vertex_handle;

  VHierarchyNodeIndex     
    node_index, lchild_node_index, rchild_node_index, 
    fund_lcut_index, fund_rcut_index;

  VHierarchyNodeHandle    
    node_handle, lchild_handle, rchild_handle;  

  std::map<VHierarchyNodeIndex, VHierarchyNodeHandle> index2handle_map;
  
  std::ifstream ifs(_filename, std::ios::binary);
  if (!ifs)
  {
    std::cerr << "read error\n";
    return  false;
  }

  //
  bool swap = OpenMesh::Endian::local() != OpenMesh::Endian::LSB;

  // read header  
  ifs.read(fileformat, 10); fileformat[10] = '\0';
  if (std::string(fileformat) != std::string("VDProgMesh"))
  {
    std::cerr << "Wrong file format.\n";
    ifs.close();
    return  false;
  }

  clear();

  OpenMesh::IO::restore(ifs, n_base_vertices_, swap);
  OpenMesh::IO::restore(ifs, n_base_faces_, swap);
  OpenMesh::IO::restore(ifs, n_details_, swap);

  // update tree_id_bits_
  vhierarchy_.set_num_roots(n_base_vertices_);


  // read base_mesh
  for (i=0; i<n_base_vertices_; ++i)
  {
    OpenMesh::IO::restore(ifs, p, swap);
    OpenMesh::IO::restore(ifs, radius, swap);
    OpenMesh::IO::restore(ifs, normal, swap);
    OpenMesh::IO::restore(ifs, sin_square, swap);
    OpenMesh::IO::restore(ifs, mue_square, swap);
    OpenMesh::IO::restore(ifs, sigma_square, swap);

    vertex_handle = add_vertex(p);
    node_index = vhierarchy_.generate_node_index(i, 1);
    node_handle = vhierarchy_.add_node();
    index2handle_map[node_index] = node_handle;

    // VHierarchyNode &node = vhierarchy_.node(node_handle);

    vhierarchy_.node(node_handle).set_index(node_index);
    vhierarchy_.node(node_handle).set_vertex_handle(vertex_handle); 
    vhierarchy_.node(node_handle).set_radius(radius);
    vhierarchy_.node(node_handle).set_normal(normal);
    vhierarchy_.node(node_handle).set_sin_square(sin_square);
    vhierarchy_.node(node_handle).set_mue_square(mue_square);
    vhierarchy_.node(node_handle).set_sigma_square(sigma_square);
  }

  for (i=0; i<n_base_faces_; ++i)
  {
    OpenMesh::IO::restore(ifs, fvi[0], swap);
    OpenMesh::IO::restore(ifs, fvi[1], swap);
    OpenMesh::IO::restore(ifs, fvi[2], swap);

    add_face(fvi);
  }

  // read details
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
    vhierarchy_.node(node_handle).set_fund_lcut(fund_lcut_index);
    vhierarchy_.node(node_handle).set_fund_rcut(fund_rcut_index);

    lchild_handle = vhierarchy_.lchild_handle(node_handle);
    rchild_handle = vhierarchy_.rchild_handle(node_handle);

    vertex_handle = add_vertex(p);
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
  set_name(_filename);

#ifdef DEBUG_COUT
  std::cout << "Opening a VDPM file has finisihed" << std::endl;
#endif

  return  true;
}

std::ostream& operator << ( std::ostream& _os, const ServerSideVDPM& _o )
{
  _os << _o.name();
  return _os;
}

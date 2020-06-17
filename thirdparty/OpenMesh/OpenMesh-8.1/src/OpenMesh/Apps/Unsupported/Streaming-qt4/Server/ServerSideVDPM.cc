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

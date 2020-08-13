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

//=============================================================================
//
//  CLASS newClass
//
//=============================================================================

#ifndef OPENMESH_VDPROGMESH_VHIERARCHY_HH
#define OPENMESH_VDPROGMESH_VHIERARCHY_HH


//== INCLUDES =================================================================

#include <vector>
#include <OpenMesh/Tools/VDPM/VHierarchyNode.hh>


//== FORWARDDECLARATIONS ======================================================


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace VDPM {

//== CLASS DEFINITION =========================================================

	      
/** Keeps the vertex hierarchy build during analyzing a progressive mesh.
 */
class OPENMESHDLLEXPORT VHierarchy
{
public:

  typedef unsigned int    id_t; ///< Type for tree and node ids

private:

  VHierarchyNodeContainer nodes_;
  unsigned int            n_roots_;
  unsigned char           tree_id_bits_; // node_id_bits_ = 32-tree_id_bits_;

public:
  
  VHierarchy();

  void clear()                        { nodes_.clear();   n_roots_ = 0; }
  unsigned char tree_id_bits() const  { return tree_id_bits_; }
  unsigned int num_roots() const      { return n_roots_; }
  size_t num_nodes() const            { return nodes_.size(); }

  VHierarchyNodeIndex generate_node_index(id_t _tree_id, id_t _node_id)
  {
    return  VHierarchyNodeIndex(_tree_id, _node_id, tree_id_bits_);
  }


  void set_num_roots(unsigned int _n_roots);
  
  VHierarchyNodeHandle root_handle(unsigned int i) const
  {
    return  VHierarchyNodeHandle( (int)i );
  }


  const VHierarchyNode& node(VHierarchyNodeHandle _vhierarchynode_handle) const
  {
    return nodes_[_vhierarchynode_handle.idx()];
  }


  VHierarchyNode& node(VHierarchyNodeHandle _vhierarchynode_handle)
  {
    return nodes_[_vhierarchynode_handle.idx()];
  }

  VHierarchyNodeHandle add_node();
  VHierarchyNodeHandle add_node(const VHierarchyNode &_node);

  void make_children(VHierarchyNodeHandle &_parent_handle);

  bool is_ancestor(VHierarchyNodeIndex _ancestor_index, 
		   VHierarchyNodeIndex _descendent_index);
  
  bool is_leaf_node(VHierarchyNodeHandle _node_handle)  
  { return nodes_[_node_handle.idx()].is_leaf(); }

  bool is_root_node(VHierarchyNodeHandle _node_handle)  
  { return nodes_[_node_handle.idx()].is_root(); }


  const OpenMesh::Vec3f& normal(VHierarchyNodeHandle _node_handle) const  
  {
    return  nodes_[_node_handle.idx()].normal(); 
  }

  const VHierarchyNodeIndex& 
  node_index(VHierarchyNodeHandle _node_handle) const
  { return  nodes_[_node_handle.idx()].node_index(); }

  VHierarchyNodeIndex& node_index(VHierarchyNodeHandle _node_handle)
  { return  nodes_[_node_handle.idx()].node_index(); }

  const VHierarchyNodeIndex& 
  fund_lcut_index(VHierarchyNodeHandle _node_handle) const
  { return  nodes_[_node_handle.idx()].fund_lcut_index(); }

  VHierarchyNodeIndex& fund_lcut_index(VHierarchyNodeHandle _node_handle)
  { return  nodes_[_node_handle.idx()].fund_lcut_index(); }

  const VHierarchyNodeIndex& 
  fund_rcut_index(VHierarchyNodeHandle _node_handle) const
  { return  nodes_[_node_handle.idx()].fund_rcut_index(); }

  VHierarchyNodeIndex& fund_rcut_index(VHierarchyNodeHandle _node_handle)
  { return  nodes_[_node_handle.idx()].fund_rcut_index(); }     
  
  VertexHandle  vertex_handle(VHierarchyNodeHandle _node_handle)
  { return  nodes_[_node_handle.idx()].vertex_handle(); }

  VHierarchyNodeHandle  parent_handle(VHierarchyNodeHandle _node_handle)
  { return nodes_[_node_handle.idx()].parent_handle(); }

  VHierarchyNodeHandle  lchild_handle(VHierarchyNodeHandle _node_handle)
  { return nodes_[_node_handle.idx()].lchild_handle(); }

  VHierarchyNodeHandle  rchild_handle(VHierarchyNodeHandle _node_handle)
  { return nodes_[_node_handle.idx()].rchild_handle(); }

  VHierarchyNodeHandle  node_handle(VHierarchyNodeIndex _node_index);

private:
  
  VHierarchyNodeHandle compute_dependency(VHierarchyNodeIndex index0, 
					  VHierarchyNodeIndex index1);

};



//=============================================================================
} // namespace VDPM
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_VDPROGMESH_VHIERARCHY_HH defined
//=============================================================================

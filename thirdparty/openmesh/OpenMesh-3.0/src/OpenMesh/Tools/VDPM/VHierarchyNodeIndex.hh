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

#ifndef OPENMESH_VDPROGMESH_VHIERARCHYNODEINDEX_HH
#define OPENMESH_VDPROGMESH_VHIERARCHYNODEINDEX_HH

//== INCLUDES =================================================================

#include <vector>
#include <cassert>

//== FORWARDDECLARATIONS ======================================================


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace VDPM {

//== CLASS DEFINITION =========================================================

	      
/** Index of vertex hierarchy node
 */


class VHierarchyNodeIndex
{
private:
  unsigned int value_;

public:

  static const VHierarchyNodeIndex  InvalidIndex;

public:

  VHierarchyNodeIndex()
  { value_ = 0; }
  
  VHierarchyNodeIndex(unsigned int _value)
  { value_ = _value; }

  VHierarchyNodeIndex(const VHierarchyNodeIndex &_other)
  { value_ = _other.value_; }

  VHierarchyNodeIndex(unsigned int   _tree_id, 
		      unsigned int   _node_id, 
		      unsigned short _tree_id_bits)
  {
    assert(_tree_id < ((unsigned int) 0x00000001 << _tree_id_bits));
    assert(_node_id < ((unsigned int) 0x00000001 << (32 - _tree_id_bits)));
    value_ = (_tree_id << (32 - _tree_id_bits)) | _node_id;
  }

  bool is_valid(unsigned short _tree_id_bits) const
  { return  node_id(_tree_id_bits) != 0 ? true : false;  }

  unsigned int tree_id(unsigned short _tree_id_bits) const
  { return  value_ >> (32 - _tree_id_bits); }
  
  unsigned int node_id(unsigned short _tree_id_bits) const
  { return  value_ & ((unsigned int) 0xFFFFFFFF >> _tree_id_bits); }

  bool operator< (const VHierarchyNodeIndex &other) const
  { return  (value_ < other.value_) ? true : false; }

  unsigned int value() const
  { return  value_; }
};


/// Container for vertex hierarchy node indices
typedef std::vector<VHierarchyNodeIndex>    VHierarchyNodeIndexContainer;


//=============================================================================
} // namespace VDPM
} // namespace OpenMesh
//=============================================================================
#endif //  OPENMESH_VDPROGMESH_VHIERARCHYNODEINDEX_HH defined
//=============================================================================

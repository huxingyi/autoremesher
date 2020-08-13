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
//  CLASS newClass - IMPLEMENTATION
//
//=============================================================================


//== INCLUDES =================================================================

#include <OpenMesh/Tools/VDPM/VFront.hh>

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace VDPM {

//== IMPLEMENTATION ========================================================== 


VFront::VFront()
{  
}


void
VFront::
add(VHierarchyNodeHandle _node_handle)
{
  front_location_[_node_handle.idx()] = front_.insert(front_.end(), _node_handle);
}


void
VFront::
remove(VHierarchyNodeHandle _node_handle)
{
  VHierarchyNodeHandleListIter node_it = front_location_[_node_handle.idx()];
  VHierarchyNodeHandleListIter next_it = front_.erase(node_it);
  front_location_[_node_handle.idx()] = front_.end();

  if (front_it_ == node_it)
    front_it_ = next_it;
}

bool
VFront::
is_active(VHierarchyNodeHandle _node_handle)
{
  return  (front_location_[_node_handle.idx()] != front_.end()) ? true : false;
}

void 
VFront::
init(VHierarchyNodeHandleContainer &_roots, unsigned int _n_details)
{
  unsigned int i;

  front_location_.resize(_roots.size() + 2*_n_details);
  for (i=0; i<front_location_.size(); ++i)
    front_location_[i] = front_.end();

  for (i=0; i<_roots.size(); ++i)
    add(_roots[i]);
}


//=============================================================================
} // namespace VDPM
} // namespace OpenMesh
//=============================================================================

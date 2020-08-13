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
//  CLASS VFront
//
//=============================================================================

#ifndef OPENMESH_VDPROGMESH_VFRONT_HH
#define OPENMESH_VDPROGMESH_VFRONT_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Tools/VDPM/VHierarchyNode.hh>
#include <vector>


//== FORWARDDECLARATIONS ======================================================


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace VDPM {

//== CLASS DEFINITION =========================================================

	      
/** Active nodes in vertex hierarchy.
    \todo VFront documentation
*/
class OPENMESHDLLEXPORT VFront
{
private:

  typedef VHierarchyNodeHandleList::iterator  VHierarchyNodeHandleListIter;
  enum VHierarchyNodeStatus { kSplit, kActive, kCollapse };
  
  VHierarchyNodeHandleList                    front_;
  VHierarchyNodeHandleListIter                front_it_;
  std::vector<VHierarchyNodeHandleListIter>   front_location_;

public:

  VFront();

  void clear() { front_.clear(); front_location_.clear(); }
  void begin() { front_it_ = front_.begin(); }
  bool end()   { return (front_it_ == front_.end()) ? true : false; }
  void next()  { ++front_it_; }
  int size()   { return (int) front_.size(); }
  VHierarchyNodeHandle node_handle()    { return  *front_it_; }

  void add(VHierarchyNodeHandle _node_handle);
  void remove(VHierarchyNodeHandle _node_handle);
  bool is_active(VHierarchyNodeHandle _node_handle);
  void init(VHierarchyNodeHandleContainer &_roots, unsigned int _n_details);  
};


//=============================================================================
} // namespace VDPM
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_VDPROGMESH_VFRONT_HH defined
//=============================================================================

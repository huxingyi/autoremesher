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
//  CLASS VDPMTraits
//
//=============================================================================


#ifndef OPENMESH_VDPM_TRAITS_HH
#define OPENMESH_VDPM_TRAITS_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Tools/VDPM/VHierarchy.hh>

//== FORWARDDECLARATIONS ======================================================


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace VDPM {

//== CLASS DEFINITION =========================================================

	      
/** \class MeshTraits MeshTraits.hh <OpenMesh/Tools/VDPM/MeshTraits.hh>

    Mesh traits for View Dependent Progressive Meshes  
*/

struct OPENMESHDLLEXPORT MeshTraits : public DefaultTraits
{
  VertexTraits
  {
  public:

    VHierarchyNodeHandle vhierarchy_node_handle()
    {
      return node_handle_; 
    }

    void set_vhierarchy_node_handle(VHierarchyNodeHandle _node_handle)
    {
      node_handle_ = _node_handle; 
    }
    
    bool is_ancestor(const VHierarchyNodeIndex &_other)
    {
      return false; 
    }

  private:

    VHierarchyNodeHandle  node_handle_;
   
  };
  
  VertexAttributes(OpenMesh::Attributes::Status |
		   OpenMesh::Attributes::Normal);
  HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
  EdgeAttributes(OpenMesh::Attributes::Status);
  FaceAttributes(OpenMesh::Attributes::Status |
		 OpenMesh::Attributes::Normal);
};


//=============================================================================
} // namespace VDPM
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_VDPM_TRAITS_HH defined
//=============================================================================


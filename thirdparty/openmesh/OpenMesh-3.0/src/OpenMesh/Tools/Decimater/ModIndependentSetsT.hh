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

/** \file ModQuadricT.hh

 */

//=============================================================================
//
//  CLASS ModQuadricT
//
//=============================================================================
#ifndef OPENMESH_TOOLS_MODINDEPENDENTSETST_HH
#define OPENMESH_TOOLS_MODINDEPENDENTSETST_HH

//== INCLUDES =================================================================

#include <OpenMesh/Tools/Decimater/ModBaseT.hh>

//== NAMESPACE ================================================================

namespace OpenMesh { // BEGIN_NS_OPENMESH
namespace Decimater { // BEGIN_NS_DECIMATER

//== CLASS DEFINITION =========================================================

/** Lock one-ring around remaining vertex after a collapse to prevent
 *  further collapses of halfedges incident to the one-ring vertices.
 */
template<class MeshT>
class ModIndependentSetsT: public ModBaseT<MeshT> {
  public:
    DECIMATING_MODULE( ModIndependentSetsT, MeshT, IndependentSets )
    ;

    /// Constructor
    ModIndependentSetsT(MeshT &_mesh) :
        Base(_mesh, true) {
    }

    /// override
    void postprocess_collapse(const CollapseInfo& _ci) {
      typename Mesh::VertexVertexIter vv_it;

      Base::mesh().status(_ci.v1).set_locked(true);
      vv_it = Base::mesh().vv_iter(_ci.v1);
      for (; vv_it.is_valid(); ++vv_it)
        Base::mesh().status(*vv_it).set_locked(true);
    }

  private:

    /// hide this method
    void set_binary(bool _b) { }
};

//=============================================================================
}// END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
#endif // OPENMESH_TOOLS_MODINDEPENDENTSETST_HH defined
//=============================================================================


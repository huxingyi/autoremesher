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
 *   $Revision$                                                        *
 *   $Date$                   *
 *                                                                           *
 \*===========================================================================*/

/** \file ModEdgeLengthT.hh
 */

//=============================================================================
//
//  CLASS ModEdgeLengthT
//
//=============================================================================
#ifndef OPENMESH_DECIMATER_MODEDGELENGTHT_HH
#define OPENMESH_DECIMATER_MODEDGELENGTHT_HH

//== INCLUDES =================================================================

#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <cfloat>

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace Decimater {

//== CLASS DEFINITION =========================================================

/** \brief Use edge length to control decimation
 *
 * This module computes the edge length.
 *
 * In binary and continuous mode, the collapse is legal if:
 *  - The length after the collapse is lower than the given tolerance
 *
 */
template<class MeshT>
class ModEdgeLengthT: public ModBaseT<MeshT> {
  public:

    DECIMATING_MODULE( ModEdgeLengthT, MeshT, EdgeLength )
    ;

    /// Constructor
    ModEdgeLengthT(MeshT& _mesh, float _edge_length = FLT_MAX,
        bool _is_binary = true);

    /// get edge_length
    float edge_length() const {
      return edge_length_;
    }

    /// set edge_length
    void set_edge_length(float _f) {
      edge_length_ = _f;
      sqr_edge_length_ = _f * _f;
    }

    /** Compute priority:
     Binary mode: Don't collapse edges longer then edge_length_
     Cont. mode:  Collapse smallest edge first, but
     don't collapse edges longer as edge_length_
     */
    float collapse_priority(const CollapseInfo& _ci);

    /// set the percentage of edge length
    void set_error_tolerance_factor(double _factor);

  private:

    Mesh& mesh_;
    typename Mesh::Scalar edge_length_, sqr_edge_length_;
};

//=============================================================================
}// END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_DECIMATER_MODEDGELENGTHT_C)
#define MODEDGELENGTHT_TEMPLATES
#include "ModEdgeLengthT.cc"
#endif
//=============================================================================
#endif // OPENMESH_DECIMATER_MODEDGELENGTHT_HH defined
//=============================================================================


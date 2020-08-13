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

/** \file ModEdgeLengthT.cc
 */

//=============================================================================
//
//  CLASS ModEdgeLengthT - IMPLEMENTATION
//
//=============================================================================
#define OPENMESH_DECIMATER_MODEDGELENGTHT_C

//== INCLUDES =================================================================

#include "ModEdgeLengthT.hh"

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace Decimater {

//== IMPLEMENTATION ==========================================================

template<class MeshT>
ModEdgeLengthT<MeshT>::ModEdgeLengthT(MeshT &_mesh, float _edge_length,
    bool _is_binary) :
    Base(_mesh, _is_binary), mesh_(Base::mesh()) {
  set_edge_length(_edge_length);
}

//-----------------------------------------------------------------------------

template<class MeshT>
float ModEdgeLengthT<MeshT>::collapse_priority(const CollapseInfo& _ci) {
  typename Mesh::Scalar sqr_length = (_ci.p0 - _ci.p1).sqrnorm();

  return ( (sqr_length <= sqr_edge_length_) ? sqr_length : float(Base::ILLEGAL_COLLAPSE));
}

//-----------------------------------------------------------------------------

template<class MeshT>
void ModEdgeLengthT<MeshT>::set_error_tolerance_factor(double _factor) {
  if (_factor >= 0.0 && _factor <= 1.0) {
    // the smaller the factor, the smaller edge_length_ gets
    // thus creating a stricter constraint
    // division by error_tolerance_factor_ is for normalization
    float edge_length = edge_length_ * _factor / this->error_tolerance_factor_;
    set_edge_length(edge_length);
    this->error_tolerance_factor_ = _factor;
  }
}

//=============================================================================
}
}
//=============================================================================

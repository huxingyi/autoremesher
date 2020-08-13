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

/** \file ModAspectRatioT.hh
 */

//=============================================================================
//
//  CLASS ModAspectRatioT
//
//=============================================================================

#ifndef OPENMESH_DECIMATER_MODASPECTRATIOT_HH
#define OPENMESH_DECIMATER_MODASPECTRATIOT_HH

//== INCLUDES =================================================================

#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <OpenMesh/Core/Utils/Property.hh>

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace Decimater {

//== CLASS DEFINITION =========================================================

/** \brief Use aspect ratio to control decimation
 *
 * This module computes the aspect ratio.
 *
 * In binary mode, the collapse is legal if:
 *  - The aspect ratio after the collapse is greater
 *  - The aspect ratio after the collapse is greater than the given minimum
 *
 * In continuous mode the collapse is illegal if:
 *  - The aspect ratio after the collapse is smaller than the given minimum
 *
 *
 */
template<class MeshT>
class ModAspectRatioT: public ModBaseT<MeshT> {
  public:

    DECIMATING_MODULE( ModAspectRatioT, MeshT, Roundness )
    ;

    typedef typename Mesh::Scalar Scalar;
    typedef typename Mesh::Point Point;

    /// constructor
    ModAspectRatioT(MeshT& _mesh, float _min_aspect = 5.0, bool _is_binary =
        true) :
        Base(_mesh, _is_binary), mesh_(Base::mesh()), min_aspect_(
            1.0 / _min_aspect) {
      mesh_.add_property(aspect_);
    }

    /// destructor
    ~ModAspectRatioT() {
      mesh_.remove_property(aspect_);
    }

    /// get aspect ratio
    float aspect_ratio() const {
      return 1.0 / min_aspect_;
    }

    /// set aspect ratio
    void set_aspect_ratio(float _f) {
      min_aspect_ = 1.0 / _f;
    }

    /// precompute face aspect ratio
    void initialize();

    /// Returns the collapse priority
    float collapse_priority(const CollapseInfo& _ci);

    /// update aspect ratio of one-ring
    void preprocess_collapse(const CollapseInfo& _ci);

    /// set percentage of aspect ratio
    void set_error_tolerance_factor(double _factor);

  private:

    /** \brief return aspect ratio (length/height) of triangle
     *
     */
    Scalar aspectRatio(const Point& _v0, const Point& _v1, const Point& _v2);

  private:

    Mesh& mesh_;
    float min_aspect_;
    FPropHandleT<float> aspect_;
};

//=============================================================================
}// END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_DECIMATER_MODASPECTRATIOT_C)
#define OPENMESH_DECIMATER_MODASPECTRATIOT_TEMPLATES
#include "ModAspectRatioT.cc"
#endif
//=============================================================================
#endif // OPENMESH_DECIMATER_MODASPECTRATIOT_HH defined
//=============================================================================


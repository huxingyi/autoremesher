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

/** \file ModHausdorffT.hh
 */

//=============================================================================
//
//  CLASS ModHausdorffT
//
//=============================================================================

#ifndef OPENMESH_DECIMATER_MODHAUSDORFFT_HH
#define OPENMESH_DECIMATER_MODHAUSDORFFT_HH

//== INCLUDES =================================================================

#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <OpenMesh/Core/Utils/Property.hh>
#include <vector>
#include <cfloat>

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace Decimater {

//== CLASS DEFINITION =========================================================

/** \brief Use Hausdorff distance to control decimation
 *
 * This module computes the aspect ratio.
 *
 * In binary mode, the collapse is legal if:
 *  - The distance after the collapse is lower than the given tolerance
 *
 * No continuous mode
 */
template<class MeshT>
class ModHausdorffT: public ModBaseT<MeshT> {
  public:

    DECIMATING_MODULE( ModHausdorffT, MeshT, Roundness );

    typedef typename Mesh::Scalar Scalar;
    typedef typename Mesh::Point Point;
    typedef typename Mesh::FaceHandle FaceHandle;
    typedef std::vector<Point> Points;

    /// Constructor
    ModHausdorffT(MeshT& _mesh, Scalar _error_tolerance = FLT_MAX) :
        Base(_mesh, true), mesh_(Base::mesh()), tolerance_(_error_tolerance) {
      mesh_.add_property(points_);
    }

    /// Destructor
    ~ModHausdorffT() {
      mesh_.remove_property(points_);
    }

    /// get max error tolerance
    Scalar tolerance() const {
      return tolerance_;
    }

    /// set max error tolerance
    void set_tolerance(Scalar _e) {
      tolerance_ = _e;
    }

    /// reset per-face point lists
    virtual void initialize();

    /** \brief compute Hausdorff error for one-ring
     *
     * This mod only allows collapses if the Hausdorff distance
     * after a collapse is lower than the given tolerance.
     *
     *
     * @param _ci Collapse info data
     * @return Binary return, if collapse is legal or illegal
     */

    virtual float collapse_priority(const CollapseInfo& _ci);

    /// re-distribute points
    virtual void postprocess_collapse(const CollapseInfo& _ci);

    /// set the percentage of tolerance
    void set_error_tolerance_factor(double _factor);

  private:

    /// squared distance from point _p to triangle (_v0, _v1, _v2)
    Scalar distPointTriangleSquared(const Point& _p, const Point& _v0, const Point& _v1, const Point& _v2);

    /// compute max error for face _fh w.r.t. its point list and _p
    Scalar compute_sqr_error(FaceHandle _fh, const Point& _p) const;

  private:

    /// Temporary point storage
    Points tmp_points_;

    Mesh&  mesh_;
    Scalar tolerance_;

    OpenMesh::FPropHandleT<Points> points_;
};

//=============================================================================
}// END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_DECIMATER_MODHAUSDORFFT_C)
#define OPENMESH_DECIMATER_MODHAUSDORFFT_TEMPLATES
#include "ModHausdorffT.cc"
#endif
//=============================================================================
#endif // OPENMESH_DECIMATER_MODHAUSDORFFT_HH defined
//=============================================================================


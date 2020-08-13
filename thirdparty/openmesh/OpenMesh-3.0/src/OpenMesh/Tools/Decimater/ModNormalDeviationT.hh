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

/** \file ModNormalDeviationT.hh
 */

//=============================================================================
//
//  CLASS ModNormalDeviationT
//
//=============================================================================


#ifndef OPENMESH_DECIMATER_MODNORMALDEVIATIONT_HH
#define OPENMESH_DECIMATER_MODNORMALDEVIATIONT_HH


//== INCLUDES =================================================================

#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <OpenMesh/Core/Utils/Property.hh>
#include <OpenMesh/Core/Geometry/NormalConeT.hh>


//== NAMESPACES ===============================================================

namespace OpenMesh  {
namespace Decimater {


//== CLASS DEFINITION =========================================================


/** \brief Use Normal deviation to control decimation
  *
  * The module tracks the normals while decimating
  * a normal cone consisting of all normals of the
  * faces collapsed together is computed and if
  * a collapse would increase the size of
  * the cone to a value greater than the given value
  * the collapse will be illegal.
  *
  * In binary and mode, the collapse is legal if:
  *  - The normal deviation after the collapse is lower than the given value
  *
  * In continuous mode the maximal deviation is returned
  */
template <class MeshT>
class ModNormalDeviationT : public ModBaseT< MeshT >
{
public:

  DECIMATING_MODULE( ModNormalDeviationT, MeshT, NormalDeviation );

  typedef typename Mesh::Scalar                     Scalar;
  typedef typename Mesh::Point                      Point;
  typedef typename Mesh::Normal                     Normal;
  typedef typename Mesh::VertexHandle               VertexHandle;
  typedef typename Mesh::FaceHandle                 FaceHandle;
  typedef typename Mesh::EdgeHandle                 EdgeHandle;
  typedef NormalConeT<Scalar>                       NormalCone;



public:

  /// Constructor
  ModNormalDeviationT(MeshT& _mesh, float _max_dev = 180.0)
  : Base(_mesh, true), mesh_(Base::mesh())
  {
    set_normal_deviation(_max_dev);
    mesh_.add_property(normal_cones_);
  }


  /// Destructor
  ~ModNormalDeviationT() {
    mesh_.add_property(normal_cones_);
  }


  /// Get normal deviation ( 0 .. 360 )
  Scalar normal_deviation() const {
    return normal_deviation_ / M_PI * 180.0;
  }

  /// Set normal deviation ( 0 .. 360 )
  void set_normal_deviation(Scalar _s) {
    normal_deviation_ = _s / 180.0 * M_PI;
  }


  /// Allocate and init normal cones
  void  initialize() {
    if (!normal_cones_.is_valid())
      mesh_.add_property(normal_cones_);

    typename Mesh::FaceIter f_it  = mesh_.faces_begin(),
        f_end = mesh_.faces_end();

    for (; f_it != f_end; ++f_it)
      mesh_.property(normal_cones_, *f_it) = NormalCone(mesh_.normal(*f_it));
  }

  /** \brief Control normals when Decimating
   *
   * Binary and Cont. mode.
   *
   * The module tracks the normals while decimating
   * a normal cone consisting of all normals of the
   * faces collapsed together is computed and if
   * a collapse would increase the size of
   * the cone to a value greater than the given value
   * the collapse will be illegal.
   *
   * @param _ci Collapse info data
   * @return Half of the normal cones size (radius in radians)
   */
  float collapse_priority(const CollapseInfo& _ci) {
    // simulate collapse
    mesh_.set_point(_ci.v0, _ci.p1);


    typename Mesh::Scalar               max_angle(0.0);
    typename Mesh::ConstVertexFaceIter  vf_it(mesh_, _ci.v0);
    typename Mesh::FaceHandle           fh, fhl, fhr;

    if (_ci.v0vl.is_valid())  fhl = mesh_.face_handle(_ci.v0vl);
    if (_ci.vrv0.is_valid())  fhr = mesh_.face_handle(_ci.vrv0);

    for (; vf_it.is_valid(); ++vf_it) {
      fh = *vf_it;
      if (fh != _ci.fl && fh != _ci.fr) {
        NormalCone nc = mesh_.property(normal_cones_, fh);

        nc.merge(NormalCone(mesh_.calc_face_normal(fh)));
        if (fh == fhl) nc.merge(mesh_.property(normal_cones_, _ci.fl));
        if (fh == fhr) nc.merge(mesh_.property(normal_cones_, _ci.fr));

        if (nc.angle() > max_angle) {
          max_angle = nc.angle();
          if (max_angle > 0.5 * normal_deviation_)
            break;
        }
      }
    }


    // undo simulation changes
    mesh_.set_point(_ci.v0, _ci.p0);


    return (max_angle < 0.5 * normal_deviation_ ? max_angle : float( Base::ILLEGAL_COLLAPSE ));
  }

  /// set the percentage of normal deviation
  void set_error_tolerance_factor(double _factor) {
    if (_factor >= 0.0 && _factor <= 1.0) {
      // the smaller the factor, the smaller normal_deviation_ gets
      // thus creating a stricter constraint
      // division by error_tolerance_factor_ is for normalization
      Scalar normal_deviation = (normal_deviation_ * 180.0/M_PI) * _factor / this->error_tolerance_factor_;
      set_normal_deviation(normal_deviation);
      this->error_tolerance_factor_ = _factor;
    }
  }


  void  postprocess_collapse(const CollapseInfo& _ci) {
    // account for changed normals
    typename Mesh::VertexFaceIter vf_it(mesh_, _ci.v1);
    for (; vf_it.is_valid(); ++vf_it)
      mesh_.property(normal_cones_, *vf_it).
      merge(NormalCone(mesh_.normal(*vf_it)));


    // normal cones of deleted triangles
    typename Mesh::FaceHandle fh;

    if (_ci.vlv1.is_valid()) {
      fh = mesh_.face_handle(mesh_.opposite_halfedge_handle(_ci.vlv1));
      if (fh.is_valid())
        mesh_.property(normal_cones_, fh).
        merge(mesh_.property(normal_cones_, _ci.fl));
    }

    if (_ci.v1vr.is_valid()) {
      fh = mesh_.face_handle(mesh_.opposite_halfedge_handle(_ci.v1vr));
      if (fh.is_valid())
        mesh_.property(normal_cones_, fh).
        merge(mesh_.property(normal_cones_, _ci.fr));
    }
  }



private:

  Mesh&                               mesh_;
  Scalar                              normal_deviation_;
  OpenMesh::FPropHandleT<NormalCone>  normal_cones_;
};


//=============================================================================
} // END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
#endif // OPENMESH_DECIMATER_MODNORMALDEVIATIONT_HH defined
//=============================================================================


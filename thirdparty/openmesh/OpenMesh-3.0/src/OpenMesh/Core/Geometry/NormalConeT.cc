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
//  CLASS NormalConeT - IMPLEMENTATION
//
//=============================================================================

#define OPENMESH_NORMALCONE_C

//== INCLUDES =================================================================

#include <math.h>
#include "NormalConeT.hh"

#ifdef max
#  undef max
#endif

#ifdef min
#  undef min
#endif


//== NAMESPACES ===============================================================


namespace OpenMesh {


//== IMPLEMENTATION ========================================================== 

template <typename Scalar>
NormalConeT<Scalar>::
NormalConeT(const Vec3& _center_normal, Scalar _angle)
  : center_normal_(_center_normal), angle_(_angle)
{
}


//----------------------------------------------------------------------------


template <typename Scalar>
Scalar 
NormalConeT<Scalar>::
max_angle(const Vec3& _norm) const
{
  Scalar dotp = (center_normal_ | _norm);
  return (dotp >= 1.0 ? 0.0 : (dotp <= -1.0 ? M_PI : acos(dotp)))
    + angle_;
}


//----------------------------------------------------------------------------


template <typename Scalar>
Scalar 
NormalConeT<Scalar>::
max_angle(const NormalConeT& _cone) const
{
  Scalar dotp = (center_normal_ | _cone.center_normal_);
  Scalar centerAngle = dotp >= 1.0 ? 0.0 : (dotp <= -1.0 ? M_PI : acos(dotp));
  Scalar sideAngle0 = std::max(angle_-centerAngle, _cone.angle_);
  Scalar sideAngle1 = std::max(_cone.angle_-centerAngle, angle_);

  return centerAngle + sideAngle0 + sideAngle1;
}


//----------------------------------------------------------------------------


template <typename Scalar>
void 
NormalConeT<Scalar>::
merge(const NormalConeT& _cone)
{
  Scalar dotp = (center_normal_ | _cone.center_normal_);

  if (fabs(dotp) < 0.99999f)
  {
    // new angle
    Scalar centerAngle = acos(dotp);
    Scalar minAngle    = std::min(-angle(), centerAngle - _cone.angle());
    Scalar maxAngle    = std::max( angle(), centerAngle + _cone.angle());
    angle_     = (maxAngle - minAngle) * Scalar(0.5f);

    // axis by SLERP
    Scalar axisAngle =  Scalar(0.5f) * (minAngle + maxAngle);
    center_normal_ = ((center_normal_ * sin(centerAngle-axisAngle)
		                   + _cone.center_normal_ * sin(axisAngle))
		                    / sin(centerAngle));
  }
  else
  {
    // axes point in same direction
    if (dotp > 0.0f)
      angle_ = std::max(angle_, _cone.angle_);

    // axes point in opposite directions
    else
      angle_ =  Scalar(2.0f * M_PI);
  }
}


//=============================================================================
} // namespace OpenMesh
//=============================================================================

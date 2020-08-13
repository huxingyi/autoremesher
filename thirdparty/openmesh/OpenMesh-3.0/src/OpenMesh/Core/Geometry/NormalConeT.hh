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
//  CLASS NormalCone
//
//=============================================================================


#ifndef OPENMESH_NORMALCONE_HH
#define OPENMESH_NORMALCONE_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/Geometry/VectorT.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {


//== CLASS DEFINITION =========================================================


/** /class NormalCone NormalCone.hh <ACG/Geometry/Types/NormalCone.hh>

    NormalCone that can be merged with other normal cones. Provides
    the center normal and the opening angle.
**/

template <typename Scalar>
class NormalConeT
{
public:

  // typedefs
  typedef VectorT<Scalar, 3>  Vec3;


  //! default constructor (not initialized)
  NormalConeT() {}

  //! Initialize cone with center (unit vector) and angle (radius in radians)
  NormalConeT(const Vec3& _center_normal, Scalar _angle=0.0);

  //! return max. distance (radians) unit vector to cone (distant side)
  Scalar max_angle(const Vec3&) const;

  //! return max. distance (radians) cone to cone (distant sides)
  Scalar max_angle(const NormalConeT&) const;

  //! merge _cone; this instance will then enclose both former cones
  void merge(const NormalConeT&);

  //! returns center normal
  const Vec3& center_normal() const { return center_normal_; }

  //! returns size of cone (radius in radians)
  inline Scalar angle() const { return angle_; }

private:

  Vec3    center_normal_;
  Scalar  angle_;
};


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_NORMALCONE_C)
#define OPENMESH_NORMALCONE_TEMPLATES
#include "NormalConeT.cc"
#endif
//=============================================================================
#endif // OPENMESH_NORMALCONE_HH defined
//=============================================================================


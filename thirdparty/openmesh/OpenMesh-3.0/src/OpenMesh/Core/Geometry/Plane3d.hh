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
//  CLASS Plane3D
//
//=============================================================================


#ifndef OPENMESH_PLANE3D_HH
#define OPENMESH_PLANE3D_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/Geometry/VectorT.hh>


//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace VDPM {

//== CLASS DEFINITION =========================================================

	      
/** \class Plane3d Plane3d.hh <OpenMesh/Tools/VDPM/Plane3d.hh>

    ax + by + cz + d = 0
*/


class OPENMESHDLLEXPORT Plane3d
{
public:

  typedef OpenMesh::Vec3f         vector_type;
  typedef vector_type::value_type value_type;

public:

  Plane3d()
    : d_(0)
  { }

  Plane3d(const vector_type &_dir, const vector_type &_pnt)
    : n_(_dir), d_(0)
  { 
    n_.normalize();
    d_ = -dot(n_,_pnt); 
  }

  value_type signed_distance(const OpenMesh::Vec3f &_p)
  {
    return  dot(n_ , _p) + d_;
  }

  // back compatibility
  value_type singed_distance(const OpenMesh::Vec3f &point)
  { return signed_distance( point ); }

public:

  vector_type n_;
  value_type  d_;

};

//=============================================================================
} // namespace VDPM
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_PLANE3D_HH defined
//=============================================================================

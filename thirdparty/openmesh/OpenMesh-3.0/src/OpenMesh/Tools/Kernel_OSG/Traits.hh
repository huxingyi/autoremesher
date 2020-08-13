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


/** \file Tools/Kernel_OSG/Traits.hh
    This file defines the default traits and some convenienve macros.
*/


//=============================================================================
//
//  CLASS Traits
//
//=============================================================================

#ifndef OPENMESH_KERNEL_OSG_TRAITS_HH
#define OPENMESH_KERNEL_OSG_TRAITS_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Tools/Kernel_OSG/VectorAdapter.hh>
//
#include <osg/Geometry>

//== NAMESPACES ===============================================================

namespace OpenMesh  {
namespace Kernel_OSG {

//== CLASS DEFINITION =========================================================


//== CLASS DEFINITION =========================================================


/** Base class for all mesh traits using the OSGArrayKernelT.
 *
 *  \see The Mesh docu section on \ref mesh_type.
 *  \see Traits.hh for a list of macros for traits classes.
 */
struct Traits : DefaultTraits
{
  typedef osg::Pnt3f            Point;
  typedef osg::Color3ub         Color;
  typedef osg::Vec3f            Normal;
  typedef osg::Vec2f            TexCoord;
  typedef osg::Vec3f::ValueType Scalar;
  
};

//=============================================================================
} // namespace Kernel_OSG
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_TRAITS_HH defined
//=============================================================================


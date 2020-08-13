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

#ifndef OPENMESH_KERNEL_OSG_COLOR_CAST_HH
#define OPENMESH_KERNEL_OSG_COLOR_CAST_HH

#include <algorithm>
#include <OpenMesh/Core/Utils/color_cast.hh>
#include <OpenSG/OSGGeometry.h>

namespace OpenMesh {

/// Helper struct
/// \internal
template <>
struct color_caster<osg::Color3ub,osg::Color3f>
{
  typedef osg::Color3ub return_type;
  typedef unsigned char ub;

  inline static return_type cast(const osg::Color3f& _src)
  {
    return return_type( (ub)std::min((_src[0]* 255.0f + 0.5f),255.0f),
                        (ub)std::min((_src[1]* 255.0f + 0.5f),255.0f),
                        (ub)std::min((_src[2]* 255.0f + 0.5f),255.0f) );
  }
};

/// Helper struct
/// \internal
template <>
struct color_caster<osg::Color3f,osg::Color3ub>
{
  typedef osg::Color3f return_type;

  inline static return_type cast(const osg::Color3ub& _src)
  {
    return return_type( (float)(_src[0] / 255.0f ),
                        (float)(_src[1] / 255.0f ),
                        (float)(_src[2] / 255.0f ) );
  }
};

} // namespace OpenMesh

#endif // OPENMESH_KERNEL_OSG_COLOR_CAST_HH 

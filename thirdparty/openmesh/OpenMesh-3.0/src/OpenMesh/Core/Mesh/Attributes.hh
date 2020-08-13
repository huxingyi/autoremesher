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


/**
    \file Attributes.hh
    This file provides some macros containing attribute usage.
*/


#ifndef OPENMESH_ATTRIBUTES_HH
#define OPENMESH_ATTRIBUTES_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Mesh/Status.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace Attributes {


//== CLASS DEFINITION  ========================================================

/** Attribute bits
 *
 *  Use the bits to define a standard property at compile time using traits.
 *
 *  \include traits5.cc
 *
 *  \see \ref mesh_type
 */
enum AttributeBits
{
  None          = 0,  ///< Clear all attribute bits
  Normal        = 1,  ///< Add normals to mesh item (vertices/faces)
  Color         = 2,  ///< Add colors to mesh item (vertices/faces/edges)
  PrevHalfedge  = 4,  ///< Add storage for previous halfedge (halfedges). The bit is set by default in the DefaultTraits.
  Status        = 8,  ///< Add status to mesh item (all items)
  TexCoord1D    = 16, ///< Add 1D texture coordinates (vertices, halfedges)
  TexCoord2D    = 32, ///< Add 2D texture coordinates (vertices, halfedges)
  TexCoord3D    = 64, ///< Add 3D texture coordinates (vertices, halfedges)
  TextureIndex  = 128 ///< Add texture index (faces)
};


//=============================================================================
} // namespace Attributes
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_ATTRIBUTES_HH defined
//=============================================================================

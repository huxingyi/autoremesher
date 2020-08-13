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
//  Helper Functions for binary reading / writing
//
//=============================================================================


#ifndef OPENMESH_VECTOR_TRAITS_HH
#define OPENMESH_VECTOR_TRAITS_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/GenProg.hh>
#if defined(OM_CC_MIPS)
#  include <stdlib.h>
#else
#  include <cstdlib>
#endif

//== NAMESPACES ===============================================================


namespace OpenMesh {


//=============================================================================


/** \name Provide a standardized access to relevant information about a
     vector type.
*/
//@{

//-----------------------------------------------------------------------------

/** Helper class providing information about a vector type.
 *
 * If want to use a different vector type than the one provided %OpenMesh
 * you need to supply a specialization of this class for the new vector type.
 */
template <typename T>
struct vector_traits
{
  /// Type of the vector class
  typedef typename T::vector_type vector_type;

  /// Type of the scalar value
  typedef typename T::value_type  value_type;

  /// size/dimension of the vector
  static const size_t size_ = T::size_;

  /// size/dimension of the vector
  static size_t size() { return size_; }
};

//@}


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_MESHREADER_HH defined
//=============================================================================

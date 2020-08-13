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

#ifndef OPENMESH_SR_BINARY_HH
#define OPENMESH_SR_BINARY_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>
// -------------------- STL
#include <typeinfo>
#include <stdexcept>
#include <sstream>
#include <numeric>   // accumulate
// -------------------- OpenMesh


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace IO {


//=============================================================================


//-----------------------------------------------------------------------------

  const static size_t UnknownSize(size_t(-1));


//-----------------------------------------------------------------------------
// struct binary, helper for storing/restoring

#define X \
   std::ostringstream msg; \
   msg << "Type not supported: " << typeid(value_type).name(); \
   throw std::logic_error(msg.str())

/// \struct binary SR_binary.hh <OpenMesh/Core/IO/SR_binary.hh>
///
/// The struct defines how to store and restore the type T.
/// It's used by the OM reader/writer modules.
///
/// The following specialization are provided:
/// - Fundamental types except \c long \c double
/// - %OpenMesh vector types
/// - %OpenMesh::StatusInfo
/// - std::string (max. length 65535)
///
/// \todo Complete documentation of members
template < typename T > struct binary
{
  typedef T     value_type;

  static const bool is_streamable = false;

  static size_t size_of(void) { return UnknownSize; }
  static size_t size_of(const value_type&) { return UnknownSize; }

  static 
  size_t store( std::ostream& /* _os */,
		const value_type& /* _v */,
		bool /* _swap=false */)
  { X; return 0; }

  static 
  size_t restore( std::istream& /* _is */,
		  value_type& /* _v */,
		  bool /* _swap=false */)
  { X; return 0; }
};

#undef X


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_SR_RBO_HH defined
//=============================================================================


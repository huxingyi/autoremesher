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
//  CLASS newClass
//
//=============================================================================


#ifndef OPENMESH_UTILS_GLCONSTASSTRING_HH
#define OPENMESH_UTILS_GLCONSTASSTRING_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>
#include <GL/glut.h>


//== FORWARDDECLARATIONS ======================================================


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace Utils {

//== CLASS DEFINITION =========================================================

inline
const char *GLenum_as_string( GLenum _m )
{
#define MODE(M) case M:return #M
  switch( _m )
  {
    MODE(GL_POINTS);
    MODE(GL_LINES);
    MODE(GL_LINE_STRIP);
    MODE(GL_LINE_LOOP);
    MODE(GL_TRIANGLES);
    MODE(GL_TRIANGLE_STRIP);
    MODE(GL_TRIANGLE_FAN);
    MODE(GL_QUADS);
    MODE(GL_QUAD_STRIP);
    MODE(GL_POLYGON);
    default: return "<unknown>";
  }
#undef MODE
}

//=============================================================================
} // namespace Utils
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_UTILS_GLCONSTASSTRING_HH defined
//=============================================================================


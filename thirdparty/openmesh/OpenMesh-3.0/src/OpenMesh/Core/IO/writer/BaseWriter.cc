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


//=== INCLUDES ================================================================


#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/IO/writer/BaseWriter.hh>
#include <algorithm>
#include <string>
#include <iterator>
#if defined(OM_CC_MIPS)
#  include <ctype.h>
#else
#  include <cctype>
#endif


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {


//=== IMPLEMENTATION ==========================================================

#ifndef DOXY_IGNORE_THIS

static inline char tolower(char c)
{
  using namespace std;
  return ::tolower(c); 
}

#endif

//-----------------------------------------------------------------------------


bool 
BaseWriter::
can_u_write(const std::string& _filename) const 
{
  // get file extension
  std::string extension;
  std::string::size_type pos(_filename.rfind("."));

  if (pos != std::string::npos)
  { 
    extension = _filename.substr(pos+1, _filename.length()-pos-1);

    std::transform( extension.begin(), extension.end(), 
		    extension.begin(), tolower );
  }

  // locate extension in extension string
  return (get_extensions().find(extension) != std::string::npos);
}


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================

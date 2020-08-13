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
//  Implements an IOManager writer module for OBJ files
//
//=============================================================================


#ifndef __OBJWRITER_HH__
#define __OBJWRITER_HH__


//=== INCLUDES ================================================================


#include <string>
#include <fstream>

#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/SingletonT.hh>
#include <OpenMesh/Core/IO/exporter/BaseExporter.hh>
#include <OpenMesh/Core/IO/writer/BaseWriter.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {


//=== IMPLEMENTATION ==========================================================


/**
    This class defines the OBJ writer. This class is further singleton'ed
    by SingletonT to OBJWriter.
*/
class OPENMESHDLLEXPORT _OBJWriter_ : public BaseWriter
{
public:

  _OBJWriter_();

  /// Destructor
  virtual ~_OBJWriter_() {};

  std::string get_description() const  { return "Alias/Wavefront"; }
  std::string get_extensions()  const  { return "obj"; }

  bool write(const std::string&, BaseExporter&, Options, std::streamsize _precision = 6) const;

  bool write(std::ostream&, BaseExporter&, Options, std::streamsize _precision = 6) const;

  size_t binary_size(BaseExporter&, Options) const { return 0; }

private:

  mutable std::string path_;
  mutable std::string objName_;

  mutable std::vector< OpenMesh::Vec3f > material_;
  mutable std::vector< OpenMesh::Vec4f > materialA_;

  size_t getMaterial(OpenMesh::Vec3f _color) const;

  size_t getMaterial(OpenMesh::Vec4f _color) const;

  bool writeMaterial(std::ostream& _out, BaseExporter&, Options) const;


};


//== TYPE DEFINITION ==========================================================


/// Declare the single entity of the OBJ writer
extern _OBJWriter_  __OBJWriterinstance;
OPENMESHDLLEXPORT _OBJWriter_& OBJWriter();


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================

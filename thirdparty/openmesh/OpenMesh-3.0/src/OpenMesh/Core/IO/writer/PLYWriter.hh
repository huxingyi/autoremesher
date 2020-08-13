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
//  Implements a writer module for PLY files
//
//=============================================================================


#ifndef __PLYWRITER_HH__
#define __PLYWRITER_HH__


//=== INCLUDES ================================================================


#include <stdio.h>
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
    Implementation of the PLY format writer. This class is singleton'ed by
    SingletonT to PLYWriter.

    currently supported options:
    - VertexColors
    - Binary
    - Binary -> MSB
*/
class OPENMESHDLLEXPORT _PLYWriter_ : public BaseWriter
{
public:

  _PLYWriter_();

  /// Destructor
  virtual ~_PLYWriter_() {};

  std::string get_description() const { return "PLY polygon file format"; }
  std::string get_extensions() const  { return "ply"; }

  bool write(const std::string&, BaseExporter&, Options, std::streamsize _precision = 6) const;

  bool write(std::ostream&, BaseExporter&, Options, std::streamsize _precision = 6) const;

  size_t binary_size(BaseExporter& _be, Options _opt) const;

  enum ValueType {
    Unsupported ,
    ValueTypeFLOAT32, ValueTypeFLOAT,
    ValueTypeUINT8, ValueTypeINT32, ValueTypeINT ,
    ValueTypeUCHAR
  };

private:
  mutable Options options_;

protected:
  void writeValue(ValueType _type, std::ostream& _out, int value) const;
  void writeValue(ValueType _type, std::ostream& _out, unsigned int value) const;
  void writeValue(ValueType _type, std::ostream& _out, float value) const;

  bool write_ascii(std::ostream& _out, BaseExporter&, Options) const;
  bool write_binary(std::ostream& _out, BaseExporter&, Options) const;
  void write_header(std::ostream& _out, BaseExporter& _be, Options& _opt) const;
};


//== TYPE DEFINITION ==========================================================


/// Declare the single entity of the PLY writer.
extern _PLYWriter_  __PLYWriterInstance;
OPENMESHDLLEXPORT _PLYWriter_& PLYWriter();


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================

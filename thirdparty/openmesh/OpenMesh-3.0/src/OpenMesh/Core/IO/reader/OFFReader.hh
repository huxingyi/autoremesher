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
//  Implements a reader module for OFF files
//
//=============================================================================


#ifndef __OFFREADER_HH__
#define __OFFREADER_HH__


//=== INCLUDES ================================================================


#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>

#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/SingletonT.hh>
#include <OpenMesh/Core/IO/reader/BaseReader.hh>

#ifndef WIN32
#include <string.h>
#endif

//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {


//== FORWARDS =================================================================


class BaseImporter;


//== IMPLEMENTATION ===========================================================


/**
    Implementation of the OFF format reader. This class is singleton'ed by
    SingletonT to OFFReader.

    By passing Options to the read function you can manipulate the reading behavoir.
    The following options can be set:

    VertexNormal
    VertexColor
    VertexTexCoord
    FaceColor
    ColorAlpha [only when reading binary]

    These options define if the corresponding data should be read (if available)
    or if it should be omitted.

    After execution of the read function. The options object contains information about
    what was actually read.

    e.g. if VertexNormal was true when the read function was called, but the file
    did not contain vertex normals then it is false afterwards.

    When reading a binary off with Color Flag in the header it is assumed that all vertices
    and faces have colors in the format "int int int".
    If ColorAlpha is set the format "int int int int" is assumed.

*/

class OPENMESHDLLEXPORT _OFFReader_ : public BaseReader
{
public:

  _OFFReader_();

  /// Destructor
  virtual ~_OFFReader_() {};

  std::string get_description() const { return "Object File Format"; }
  std::string get_extensions()  const { return "off"; }
  std::string get_magic()       const { return "OFF"; }

  bool read(const std::string& _filename,
	    BaseImporter& _bi,
	    Options& _opt);

  bool can_u_read(const std::string& _filename) const;

  bool read(std::istream& _in, BaseImporter& _bi, Options& _opt );

private:

  bool can_u_read(std::istream& _is) const;

  bool read_ascii(std::istream& _in, BaseImporter& _bi, Options& _opt) const;
  bool read_binary(std::istream& _in, BaseImporter& _bi, Options& _opt, bool swap) const;

  void readValue(std::istream& _in, float& _value) const;
  void readValue(std::istream& _in, int& _value) const;
  void readValue(std::istream& _in, unsigned int& _value) const;

  int getColorType(std::string & _line, bool _texCoordsAvailable) const;

  //available options for reading
  mutable Options options_;
  //options that the user wants to read
  mutable Options userOptions_;
};


//== TYPE DEFINITION ==========================================================


/// Declare the single entity of the OFF reader
extern _OFFReader_  __OFFReaderInstance;
OPENMESHDLLEXPORT _OFFReader_& OFFReader();


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================

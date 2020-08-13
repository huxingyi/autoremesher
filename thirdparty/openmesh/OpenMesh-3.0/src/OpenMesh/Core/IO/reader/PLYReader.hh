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


#ifndef __PLYREADER_HH__
#define __PLYREADER_HH__


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
    Implementation of the PLY format reader. This class is singleton'ed by
    SingletonT to OFFReader.

*/

class OPENMESHDLLEXPORT _PLYReader_ : public BaseReader
{
public:

  _PLYReader_();

  std::string get_description() const { return "PLY polygon file format"; }
  std::string get_extensions()  const { return "ply"; }
  std::string get_magic()       const { return "PLY"; }

  bool read(const std::string& _filename,
        BaseImporter& _bi,
        Options& _opt);

  bool read(std::istream& _is,
            BaseImporter& _bi,
            Options& _opt);

  bool can_u_read(const std::string& _filename) const;

  enum ValueType {
    Unsupported,
    ValueTypeINT8, ValueTypeCHAR,
    ValueTypeUINT8, ValueTypeUCHAR,
    ValueTypeINT16, ValueTypeSHORT,
    ValueTypeUINT16, ValueTypeUSHORT,
    ValueTypeINT32, ValueTypeINT,
    ValueTypeUINT32, ValueTypeUINT,
    ValueTypeFLOAT32, ValueTypeFLOAT,
    ValueTypeFLOAT64, ValueTypeDOUBLE
  };

private:

  bool can_u_read(std::istream& _is) const;

  bool read_ascii(std::istream& _in, BaseImporter& _bi, const Options& _opt) const;
  bool read_binary(std::istream& _in, BaseImporter& _bi, bool swap, const Options& _opt) const;

  float readToFloatValue(ValueType _type , std::fstream& _in) const;

  void readValue(ValueType _type , std::istream& _in, float& _value) const;
  void readValue(ValueType _type, std::istream& _in, double& _value) const;
  void readValue(ValueType _type , std::istream& _in, unsigned int& _value) const;
  void readValue(ValueType _type , std::istream& _in, int& _value) const;

  void readInteger(ValueType _type, std::istream& _in, int& _value) const;
  void readInteger(ValueType _type, std::istream& _in, unsigned int& _value) const;

  /// Read unsupported properties in PLY file
  void consume_input(std::istream& _in, int _count) const {
	  _in.read(reinterpret_cast<char*>(&buff[0]), _count);
  }

  mutable unsigned char buff[8];

  /// Available per file options for reading
  mutable Options options_;

  /// Options that the user wants to read
  mutable Options userOptions_;

  mutable unsigned int vertexCount_;
  mutable unsigned int faceCount_;

  mutable ValueType vertexType_;
  mutable uint vertexDimension_;

  mutable ValueType faceIndexType_;
  mutable ValueType faceEntryType_;

  enum VertexProperty {
    XCOORD,YCOORD,ZCOORD,
    TEXX,TEXY,
    COLORRED,COLORGREEN,COLORBLUE,COLORALPHA,
    XNORM,YNORM,ZNORM,
    UNSUPPORTED
  };

  /// Stores sizes of property types
  mutable std::map<ValueType, int> scalar_size_;

  // Number of vertex properties
  mutable unsigned int vertexPropertyCount_;
  mutable std::map< int , std::pair< VertexProperty, ValueType> > vertexPropertyMap_;

};


//== TYPE DEFINITION ==========================================================


/// Declare the single entity of the PLY reader
extern _PLYReader_  __PLYReaderInstance;
OPENMESHDLLEXPORT _PLYReader_&  PLYReader();


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================

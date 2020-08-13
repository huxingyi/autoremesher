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
//  Implements a writer module for OM files
//
//=============================================================================


#ifndef __OMWRITER_HH__
#define __OMWRITER_HH__


//=== INCLUDES ================================================================


// STD C++
#include <iostream>
#include <string>

// OpenMesh
#include <OpenMesh/Core/IO/BinaryHelper.hh>
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/SingletonT.hh>
#include <OpenMesh/Core/IO/OMFormat.hh>
#include <OpenMesh/Core/IO/IOManager.hh>
#include <OpenMesh/Core/IO/writer/BaseWriter.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {

//=== FORWARDS ================================================================


class BaseExporter;


//=== IMPLEMENTATION ==========================================================


/**
 *  Implementation of the OM format writer. This class is singleton'ed by
 *  SingletonT to OMWriter.
 */
class OPENMESHDLLEXPORT _OMWriter_ : public BaseWriter
{
public:

  /// Constructor
  _OMWriter_();

  /// Destructor
  virtual ~_OMWriter_() {};

  std::string get_description() const
  { return "OpenMesh Format"; }

  std::string get_extensions() const
  { return "om"; }

  bool write(std::ostream&, BaseExporter&, Options, std::streamsize _precision = 6) const;



  size_t binary_size(BaseExporter& _be, Options _opt) const;


protected:

  static const OMFormat::uchar magic_[3];
  static const OMFormat::uint8 version_;

  bool write(const std::string&, BaseExporter&, Options, std::streamsize _precision = 6) const;

  bool write_binary(std::ostream&, BaseExporter&, Options) const;


  size_t store_binary_custom_chunk( std::ostream&, const BaseProperty&,
				    OMFormat::Chunk::Entity, bool) const;
};


//== TYPE DEFINITION ==========================================================


/// Declare the single entity of the OM writer.
extern _OMWriter_  __OMWriterInstance;
OPENMESHDLLEXPORT _OMWriter_& OMWriter();


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================

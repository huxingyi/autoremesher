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


#ifndef __OMREADER_HH__
#define __OMREADER_HH__


//=== INCLUDES ================================================================

// OpenMesh
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/SingletonT.hh>
#include <OpenMesh/Core/IO/OMFormat.hh>
#include <OpenMesh/Core/IO/IOManager.hh>
#include <OpenMesh/Core/IO/importer/BaseImporter.hh>
#include <OpenMesh/Core/IO/reader/BaseReader.hh>

// STD C++
#include <iostream>
#include <string>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {


//== IMPLEMENTATION ===========================================================


/**
    Implementation of the OM format reader. This class is singleton'ed by
    SingletonT to OMReader.
*/
class OPENMESHDLLEXPORT _OMReader_ : public BaseReader
{
public:

  _OMReader_();
  virtual ~_OMReader_() { }

  std::string get_description() const { return "OpenMesh File Format"; }
  std::string get_extensions()  const { return "om"; }
  std::string get_magic()       const { return "OM"; }

  bool read(const std::string& _filename,
	    BaseImporter& _bi,
	    Options& _opt );

//!  Stream Reader for std::istream input in binary format
  bool read(std::istream& _is,
	    BaseImporter& _bi,
	    Options& _opt );

  virtual bool can_u_read(const std::string& _filename) const;
  virtual bool can_u_read(std::istream& _is) const;


private:

  bool supports( const OMFormat::uint8 version ) const;

  bool read_ascii(std::istream& _is, BaseImporter& _bi, Options& _opt) const;
  bool read_binary(std::istream& _is, BaseImporter& _bi, Options& _opt) const;

  typedef OMFormat::Header              Header;
  typedef OMFormat::Chunk::Header       ChunkHeader;
  typedef OMFormat::Chunk::PropertyName PropertyName;

  // initialized/updated by read_binary*/read_ascii*
  mutable size_t       bytes_;
  mutable Options      fileOptions_;
  mutable Header       header_;
  mutable ChunkHeader  chunk_header_;
  mutable PropertyName property_name_;

  bool read_binary_vertex_chunk(   std::istream      &_is,
				   BaseImporter      &_bi,
				   Options           &_opt,
				   bool              _swap) const;

  bool read_binary_face_chunk(     std::istream      &_is,
			           BaseImporter      &_bi,
			           Options           &_opt,
				   bool              _swap) const;

  bool read_binary_edge_chunk(     std::istream      &_is,
			           BaseImporter      &_bi,
			           Options           &_opt,
				   bool              _swap) const;

  bool read_binary_halfedge_chunk( std::istream      &_is,
				   BaseImporter      &_bi,
				   Options           &_opt,
				   bool              _swap) const;

  bool read_binary_mesh_chunk(     std::istream      &_is,
				   BaseImporter      &_bi,
				   Options           &_opt,
				   bool              _swap) const;

  size_t restore_binary_custom_data( std::istream& _is,
				     BaseProperty* _bp,
				     size_t _n_elem,
				     bool _swap) const;

};


//== TYPE DEFINITION ==========================================================


/// Declare the single entity of the OM reader.
extern _OMReader_  __OMReaderInstance;
OPENMESHDLLEXPORT _OMReader_&  OMReader();


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================

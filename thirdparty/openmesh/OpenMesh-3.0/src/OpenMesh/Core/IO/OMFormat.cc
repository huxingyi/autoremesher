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

//== INCLUDES =================================================================

#include <OpenMesh/Core/IO/OMFormat.hh>
#include <algorithm>
#include <iomanip>

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace IO {
namespace OMFormat {

//== IMPLEMENTATION ===========================================================

  Chunk::Integer_Size needed_bits( size_t s )
  {
    if (s <= 0x000100) return Chunk::Integer_8;
    if (s <= 0x010000) return Chunk::Integer_16;

#if 0
    // !Not tested yet! This most probably won't work!
    // NEED a 64bit system!
    if ( (sizeof( size_t ) == 8) && (s >= 0x100000000) )
      return Chunk::Integer_64;
#endif

    return Chunk::Integer_32;    
  }

//-----------------------------------------------------------------------------

  uint16& 
  operator << (uint16& val, const Chunk::Header& hdr)
  {
    val = 0;
    val |= hdr.name_   << OMFormat::Chunk::OFF_NAME;
    val |= hdr.entity_ << OMFormat::Chunk::OFF_ENTITY;
    val |= hdr.type_   << OMFormat::Chunk::OFF_TYPE;  
    val |= hdr.signed_ << OMFormat::Chunk::OFF_SIGNED;
    val |= hdr.float_  << OMFormat::Chunk::OFF_FLOAT; 
    val |= hdr.dim_    << OMFormat::Chunk::OFF_DIM;   
    val |= hdr.bits_   << OMFormat::Chunk::OFF_BITS;  
    return val;
  }


//-----------------------------------------------------------------------------

  Chunk::Header&
  operator << (Chunk::Header& hdr, const uint16 val)
  {
    hdr.reserved_ = 0;
    hdr.name_     = val >> OMFormat::Chunk::OFF_NAME;
    hdr.entity_   = val >> OMFormat::Chunk::OFF_ENTITY;
    hdr.type_     = val >> OMFormat::Chunk::OFF_TYPE;
    hdr.signed_   = val >> OMFormat::Chunk::OFF_SIGNED;
    hdr.float_    = val >> OMFormat::Chunk::OFF_FLOAT;
    hdr.dim_      = val >> OMFormat::Chunk::OFF_DIM;
    hdr.bits_     = val >> OMFormat::Chunk::OFF_BITS;
    return hdr;
  }

//-----------------------------------------------------------------------------

  const char *as_string(Chunk::Entity e)
  {
    switch(e)
    {    
      case Chunk::Entity_Vertex:   return "Vertex";
      case Chunk::Entity_Mesh:     return "Mesh";
      case Chunk::Entity_Edge:     return "Edge";
      case Chunk::Entity_Halfedge: return "Halfedge";
      case Chunk::Entity_Face:     return "Face";
      default:
	std::clog << "as_string(Chunk::Entity): Invalid value!";
    }
    return NULL;
  }


//-----------------------------------------------------------------------------

  const char *as_string(Chunk::Type t)
  {
    switch(t)
    {    
      case Chunk::Type_Pos:      return "Pos";
      case Chunk::Type_Normal:   return "Normal";
      case Chunk::Type_Texcoord: return "Texcoord";
      case Chunk::Type_Status:   return "Status";
      case Chunk::Type_Color:    return "Color";
      case Chunk::Type_Custom:   return "Custom";
      case Chunk::Type_Topology: return "Topology";
    }
    return NULL;
  }


//-----------------------------------------------------------------------------

  const char *as_string(Chunk::Dim d)
  {
    switch(d)
    {    
      case Chunk::Dim_1D: return "1D";
      case Chunk::Dim_2D: return "2D";
      case Chunk::Dim_3D: return "3D";
      case Chunk::Dim_4D: return "4D";
      case Chunk::Dim_5D: return "5D";
      case Chunk::Dim_6D: return "6D";
      case Chunk::Dim_7D: return "7D";
      case Chunk::Dim_8D: return "8D";
    }
    return NULL;
  }


//-----------------------------------------------------------------------------

  const char *as_string(Chunk::Integer_Size d)
  {
    switch(d)
    {    
      case Chunk::Integer_8  : return "8";
      case Chunk::Integer_16 : return "16";
      case Chunk::Integer_32 : return "32";
      case Chunk::Integer_64 : return "64";
    }
    return NULL;
  }

  const char *as_string(Chunk::Float_Size d)
  {
    switch(d)
    {    
      case Chunk::Float_32 : return "32";
      case Chunk::Float_64 : return "64";
      case Chunk::Float_128: return "128";
    }
    return NULL;
  }


//-----------------------------------------------------------------------------

  std::ostream& operator << ( std::ostream& _os, const Chunk::Header& _c )
  {
    _os << "Chunk Header : 0x" << std::setw(4)
	<< std::hex << (*(uint16*)(&_c)) << std::dec << std::endl;
    _os << "entity = " 
	<< as_string(Chunk::Entity(_c.entity_)) << std::endl;
    _os << "type   = " 
	<< as_string(Chunk::Type(_c.type_));
    if ( Chunk::Type(_c.type_)!=Chunk::Type_Custom) 
    {
      _os << std::endl 
	  << "signed = " 
	  << _c.signed_ << std::endl;
      _os << "float  = " 
	  << _c.float_ << std::endl;
      _os << "dim    = " 
	  << as_string(Chunk::Dim(_c.dim_)) << std::endl;
      _os << "bits   = " 
	  << (_c.float_
	      ? as_string(Chunk::Float_Size(_c.bits_)) 
	      : as_string(Chunk::Integer_Size(_c.bits_)));
    }
    return _os;
  }


//-----------------------------------------------------------------------------

  std::ostream& operator << ( std::ostream& _os, const Header& _h )
  {
    _os << "magic   = '" << _h.magic_[0] << _h.magic_[1] << "'\n"
	<< "mesh    = '" << _h.mesh_ << "'\n"
	<< "version = 0x" << std::hex << (uint16)_h.version_ << std::dec
	<< " (" << major_version(_h.version_) 
	<< "."  << minor_version(_h.version_) << ")\n"
	<< "#V      = " << _h.n_vertices_ << std::endl
	<< "#F      = " << _h.n_faces_ << std::endl
	<< "#E      = " << _h.n_edges_;
    return _os;
  }


} // namespace OMFormat
  // --------------------------------------------------------------------------

//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================

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


#define OPENMESH_IO_OMFORMAT_CC


//== INCLUDES =================================================================

#include <OpenMesh/Core/IO/OMFormat.hh>
#include <algorithm>
#include <iomanip>

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace IO {

  // helper to store a an integer
  template< typename T > 
  size_t 
  store( std::ostream& _os, 
	 const T& _val, 
	 OMFormat::Chunk::Integer_Size _b, 
	 bool _swap,
	 t_signed)
  {    
    assert( OMFormat::is_integer( _val ) );

    switch( _b ) 
    {
      case OMFormat::Chunk::Integer_8:
      { 	
	OMFormat::int8 v = static_cast<OMFormat::int8>(_val);
	return store( _os, v, _swap );
      }
      case OMFormat::Chunk::Integer_16:
      { 
	OMFormat::int16 v = static_cast<OMFormat::int16>(_val);
	return store( _os, v, _swap );
      }
      case OMFormat::Chunk::Integer_32:
      { 
	OMFormat::int32 v = static_cast<OMFormat::int32>(_val);
	return store( _os, v, _swap );
      }      
      case OMFormat::Chunk::Integer_64:
      { 
	OMFormat::int64 v = static_cast<OMFormat::int64>(_val);
	return store( _os, v, _swap );
      }
    }
    return 0;
  }


  // helper to store a an unsigned integer
  template< typename T > 
  size_t 
  store( std::ostream& _os, 
	 const T& _val, 
	 OMFormat::Chunk::Integer_Size _b, 
	 bool _swap,
	 t_unsigned)
  {    
    assert( OMFormat::is_integer( _val ) );

    switch( _b ) 
    {
      case OMFormat::Chunk::Integer_8:
      { 
	OMFormat::uint8 v = static_cast<OMFormat::uint8>(_val);
	return store( _os, v, _swap );
      }
      case OMFormat::Chunk::Integer_16:
      { 
	OMFormat::uint16 v = static_cast<OMFormat::uint16>(_val);
	return store( _os, v, _swap );
      }
      case OMFormat::Chunk::Integer_32:
      { 
	OMFormat::uint32 v = static_cast<OMFormat::uint32>(_val);
	return store( _os, v, _swap );
      }
      
      case OMFormat::Chunk::Integer_64:
      { 
	OMFormat::uint64 v = static_cast<OMFormat::uint64>(_val);
	return store( _os, v, _swap );
      }
    }
    return 0;
  }


  // helper to store a an integer
  template< typename T > 
  size_t 
  restore( std::istream& _is, 
	   T& _val, 
	   OMFormat::Chunk::Integer_Size _b, 
	   bool _swap,
	   t_signed)
  {    
    assert( OMFormat::is_integer( _val ) );
    size_t bytes = 0;

    switch( _b ) 
    {
      case OMFormat::Chunk::Integer_8:
      { 	
	OMFormat::int8 v;
	bytes = restore( _is, v, _swap );
	_val = static_cast<T>(v);
	break;
      }
      case OMFormat::Chunk::Integer_16:
      { 
	OMFormat::int16 v;
	bytes = restore( _is, v, _swap );
	_val = static_cast<T>(v);
      }
      case OMFormat::Chunk::Integer_32:
      { 
	OMFormat::int32 v;
	bytes = restore( _is, v, _swap );
	_val = static_cast<T>(v);
      }      
      case OMFormat::Chunk::Integer_64:
      { 
	OMFormat::int64 v;
	bytes = restore( _is, v, _swap );
	_val = static_cast<T>(v);
      }
    }
    return bytes;
  }


  // helper to store a an unsigned integer
  template< typename T > 
  size_t 
  restore( std::istream& _is, 
	   T& _val, 
	   OMFormat::Chunk::Integer_Size _b, 
	   bool _swap,
	   t_unsigned)
  {    
    assert( OMFormat::is_integer( _val ) );
    size_t bytes = 0;

    switch( _b ) 
    {
      case OMFormat::Chunk::Integer_8:
      { 
	OMFormat::uint8 v;
	bytes = restore( _is, v, _swap );
	_val = static_cast<T>(v);
	break;
      }
      case OMFormat::Chunk::Integer_16:
      { 
	OMFormat::uint16 v;
	bytes = restore( _is, v, _swap );
	_val = static_cast<T>(v);
	break;
      }
      case OMFormat::Chunk::Integer_32:
      { 
	OMFormat::uint32 v;
	bytes = restore( _is, v, _swap );
	_val = static_cast<T>(v);
	break;
      }
      
      case OMFormat::Chunk::Integer_64:
      { 
	OMFormat::uint64 v;
	bytes = restore( _is, v, _swap );
	_val = static_cast<T>(v);
	break;
      }
    }
    return bytes;
  }

//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================

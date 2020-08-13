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


#ifndef OPENMESH_VECTORCAST_HH
#define OPENMESH_VECTORCAST_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/vector_traits.hh>
#include <OpenMesh/Core/Utils/GenProg.hh>
#include <iostream>
#include <algorithm>
#include <OpenMesh/Core/Geometry/VectorT.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {


//=============================================================================


/** \name Cast vector type to another vector type.
*/
//@{

//-----------------------------------------------------------------------------


template <typename src_t, typename dst_t>
inline void vector_copy( const src_t &_src, dst_t &_dst, GenProg::Int2Type<1> )
{
  _dst[0] = _src[0];
}

template <typename src_t, typename dst_t>
inline void vector_copy( const src_t &_src, dst_t &_dst, GenProg::Int2Type<2> )
{
  _dst[0] = _src[0];
  _dst[1] = _src[1];
}

template <typename src_t, typename dst_t>
inline void vector_copy( const src_t &_src, dst_t &_dst, GenProg::Int2Type<3> )
{
  _dst[0] = _src[0];
  _dst[1] = _src[1];
  _dst[2] = _src[2];
}

template <typename src_t, typename dst_t>
inline void vector_copy( const src_t &_src, dst_t &_dst, GenProg::Int2Type<4> )
{
  _dst[0] = _src[0];
  _dst[1] = _src[1];
  _dst[2] = _src[2];
  _dst[3] = _src[3];
}

template <typename src_t, typename dst_t>
inline void vector_copy( const src_t &_src, dst_t &_dst, GenProg::Int2Type<5> )
{
  _dst[0] = _src[0];
  _dst[1] = _src[1];
  _dst[2] = _src[2];
  _dst[3] = _src[3];
  _dst[4] = _src[4];
}

template <typename src_t, typename dst_t>
inline void vector_copy( const src_t &_src, dst_t &_dst, GenProg::Int2Type<6> )
{
  _dst[0] = _src[0];
  _dst[1] = _src[1];
  _dst[2] = _src[2];
  _dst[3] = _src[3];
  _dst[4] = _src[4];
  _dst[5] = _src[5];
}


//-----------------------------------------------------------------------------
#ifndef DOXY_IGNORE_THIS

template <typename dst_t, typename src_t>
struct vector_caster
{
  typedef dst_t  return_type;

  inline static return_type cast(const src_t& _src)
  {
    dst_t dst;
    vector_copy(_src, dst, GenProg::Int2Type<vector_traits<dst_t>::size_>());
    return dst;
  }
};

#if !defined(OM_CC_MSVC)
template <typename dst_t>
struct vector_caster<dst_t,dst_t>
{
  typedef const dst_t&  return_type;

  inline static return_type cast(const dst_t& _src)
  {
    return _src;
  }
};
#endif

#endif
//-----------------------------------------------------------------------------


/// Cast vector type to another vector type by copying the vector elements
template <typename dst_t, typename src_t>
inline
typename vector_caster<dst_t, src_t>::return_type
vector_cast(const src_t& _src )
{
  return vector_caster<dst_t, src_t>::cast(_src);
}


//@}


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_MESHREADER_HH defined
//=============================================================================

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

#ifndef OPENMESH_SR_RBO_HH
#define OPENMESH_SR_RBO_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>
// -------------------- STL
#if defined(OM_CC_MIPS)
#  include <stdio.h> // size_t
#else
#  include <cstdio>  // size_t
#endif
#include <algorithm>
#include <typeinfo>
// -------------------- OpenMesh
#include <OpenMesh/Core/System/omstream.hh>
#include <OpenMesh/Core/IO/SR_types.hh>
#include <OpenMesh/Core/Utils/GenProg.hh>

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace IO {


//=============================================================================


/** \name Handling binary input/output.
    These functions take care of swapping bytes to get the right Endian.
*/
//@{


//-----------------------------------------------------------------------------

/** this does not compile for g++3.4 and higher, hence we comment the
function body which will result in a linker error */

template < size_t N > inline
void _reverse_byte_order_N(uint8_t* _val);

template <> inline
void _reverse_byte_order_N<1>(uint8_t* /*_val*/) { }


template <> inline
void _reverse_byte_order_N<2>(uint8_t* _val)
{
   _val[0] ^= _val[1]; _val[1] ^= _val[0]; _val[0] ^= _val[1];
}


template <> inline
void _reverse_byte_order_N<4>(uint8_t* _val)
{
   _val[0] ^= _val[3]; _val[3] ^= _val[0]; _val[0] ^= _val[3]; // 0 <-> 3
   _val[1] ^= _val[2]; _val[2] ^= _val[1]; _val[1] ^= _val[2]; // 1 <-> 2
}


template <> inline
void _reverse_byte_order_N<8>(uint8_t* _val)
{
   _val[0] ^= _val[7]; _val[7] ^= _val[0]; _val[0] ^= _val[7]; // 0 <-> 7
   _val[1] ^= _val[6]; _val[6] ^= _val[1]; _val[1] ^= _val[6]; // 1 <-> 6
   _val[2] ^= _val[5]; _val[5] ^= _val[2]; _val[2] ^= _val[5]; // 2 <-> 5
   _val[3] ^= _val[4]; _val[4] ^= _val[3]; _val[3] ^= _val[4]; // 3 <-> 4
}


template <> inline
void _reverse_byte_order_N<12>(uint8_t* _val)
{
   _val[0] ^= _val[11]; _val[11] ^= _val[0]; _val[0] ^= _val[11]; // 0 <-> 11
   _val[1] ^= _val[10]; _val[10] ^= _val[1]; _val[1] ^= _val[10]; // 1 <-> 10
   _val[2] ^= _val[ 9]; _val[ 9] ^= _val[2]; _val[2] ^= _val[ 9]; // 2 <->  9
   _val[3] ^= _val[ 8]; _val[ 8] ^= _val[3]; _val[3] ^= _val[ 8]; // 3 <->  8
   _val[4] ^= _val[ 7]; _val[ 7] ^= _val[4]; _val[4] ^= _val[ 7]; // 4 <->  7
   _val[5] ^= _val[ 6]; _val[ 6] ^= _val[5]; _val[5] ^= _val[ 6]; // 5 <->  6
}


template <> inline
void _reverse_byte_order_N<16>(uint8_t* _val)
{
   _reverse_byte_order_N<8>(_val);
   _reverse_byte_order_N<8>(_val+8);
   std::swap(*(uint64_t*)_val, *(((uint64_t*)_val)+1));
}


//-----------------------------------------------------------------------------
// wrapper for byte reordering

// reverting pointers makes no sense, hence forbid it.
/** this does not compile for g++3.4 and higher, hence we comment the
function body which will result in a linker error */
template <typename T> inline T* reverse_byte_order(T* t);
// Should never reach this point. If so, then some operator were not
// overloaded. Especially check for IO::binary<> specialization on
// custom data types.


inline void compile_time_error__no_fundamental_type()
{
  // we should never reach this point
  assert(false);
}

// default action for byte reversal: cause an error to avoid
// surprising behaviour!
template <typename T> T& reverse_byte_order(  T& _t )
{
  omerr() << "Not defined for type " << typeid(T).name() << std::endl;
  compile_time_error__no_fundamental_type();
  return _t;
}

template <> inline bool&  reverse_byte_order(bool & _t) { return _t; }
template <> inline char&  reverse_byte_order(char & _t) { return _t; }
#if defined(OM_CC_GCC)
template <> inline signed char&  reverse_byte_order(signed char & _t) { return _t; }
#endif
template <> inline uchar& reverse_byte_order(uchar& _t) { return _t; }

// Instead do specializations for the necessary types
#define REVERSE_FUNDAMENTAL_TYPE( T ) \
  template <> inline T& reverse_byte_order( T&  _t ) {\
   _reverse_byte_order_N<sizeof(T)>( reinterpret_cast<uint8_t*>(&_t) ); \
   return _t; \
  }

// REVERSE_FUNDAMENTAL_TYPE(bool)
// REVERSE_FUNDAMENTAL_TYPE(char)
// REVERSE_FUNDAMENTAL_TYPE(uchar)
REVERSE_FUNDAMENTAL_TYPE(int16_t)
REVERSE_FUNDAMENTAL_TYPE(uint16_t)
// REVERSE_FUNDAMENTAL_TYPE(int)
// REVERSE_FUNDAMENTAL_TYPE(uint)

REVERSE_FUNDAMENTAL_TYPE(unsigned long)
REVERSE_FUNDAMENTAL_TYPE(int32_t)
REVERSE_FUNDAMENTAL_TYPE(uint32_t)
REVERSE_FUNDAMENTAL_TYPE(int64_t)
REVERSE_FUNDAMENTAL_TYPE(uint64_t)
REVERSE_FUNDAMENTAL_TYPE(float)
REVERSE_FUNDAMENTAL_TYPE(double)
REVERSE_FUNDAMENTAL_TYPE(long double)

#undef REVERSE_FUNDAMENTAL_TYPE

#if 0

#define REVERSE_VECTORT_TYPE( T ) \
  template <> inline T& reverse_byte_order(T& _v) {\
    for (size_t i; i< T::size_; ++i) \
      _reverse_byte_order_N< sizeof(T::value_type) >( reinterpret_cast<uint8_t*>(&_v[i])); \
    return _v; \
  }

#define REVERSE_VECTORT_TYPES( N )  \
  REVERSE_VECTORT_TYPE( Vec##N##c )  \
  REVERSE_VECTORT_TYPE( Vec##N##uc ) \
  REVERSE_VECTORT_TYPE( Vec##N##s )  \
  REVERSE_VECTORT_TYPE( Vec##N##us ) \
  REVERSE_VECTORT_TYPE( Vec##N##i )  \
  REVERSE_VECTORT_TYPE( Vec##N##ui ) \
  REVERSE_VECTORT_TYPE( Vec##N##f )  \
  REVERSE_VECTORT_TYPE( Vec##N##d )  \

REVERSE_VECTORT_TYPES(1)
REVERSE_VECTORT_TYPES(2)
REVERSE_VECTORT_TYPES(3)
REVERSE_VECTORT_TYPES(4)
REVERSE_VECTORT_TYPES(6)

#undef REVERSE_VECTORT_TYPES
#undef REVERSE_VECTORT_TYPE

#endif

template <typename T> inline
T reverse_byte_order(const T& a)
{
  compile_timer_error__const_means_const(a);
  return a;
}


//@}


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_SR_RBO_HH defined
//=============================================================================


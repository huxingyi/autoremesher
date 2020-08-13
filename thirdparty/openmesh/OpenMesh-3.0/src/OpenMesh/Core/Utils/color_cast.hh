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


#ifndef OPENMESH_COLOR_CAST_HH
#define OPENMESH_COLOR_CAST_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/vector_cast.hh>

//== NAMESPACES ===============================================================


namespace OpenMesh {


//=============================================================================


/** \name Cast vector type to another vector type.
*/
//@{

//-----------------------------------------------------------------------------
#ifndef DOXY_IGNORE_THIS

/// Cast one color vector to another.
template <typename dst_t, typename src_t>
struct color_caster
{
  typedef dst_t  return_type;

  inline static return_type cast(const src_t& _src)
  {
    dst_t dst;
    vector_copy(_src, dst, GenProg::Int2Type<vector_traits<dst_t>::size_>());
    return dst;
  }
};


template <>
struct color_caster<Vec3uc,Vec3f>
{
  typedef Vec3uc return_type;

  inline static return_type cast(const Vec3f& _src)
  {
    return Vec3uc( (unsigned char)(_src[0]* 255.0f + 0.5f),
		               (unsigned char)(_src[1]* 255.0f + 0.5f),
		               (unsigned char)(_src[2]* 255.0f + 0.5f) );
  }
};

template <>
struct color_caster<Vec3uc,Vec4f>
{
  typedef Vec3uc return_type;

  inline static return_type cast(const Vec4f& _src)
  {
    return Vec3uc( (unsigned char)(_src[0]* 255.0f + 0.5f),
                   (unsigned char)(_src[1]* 255.0f + 0.5f),
                   (unsigned char)(_src[2]* 255.0f + 0.5f) );
  }
};

template <>
struct color_caster<Vec3i,Vec3f>
{
  typedef Vec3i return_type;

  inline static return_type cast(const Vec3f& _src)
  {
    return Vec3i( (int)(_src[0]* 255.0f + 0.5f),
                  (int)(_src[1]* 255.0f + 0.5f),
                  (int)(_src[2]* 255.0f + 0.5f) );
  }
};

template <>
struct color_caster<Vec3i,Vec4f>
{
  typedef Vec3i return_type;

  inline static return_type cast(const Vec4f& _src)
  {
    return Vec3i( (int)(_src[0]* 255.0f + 0.5f),
                  (int)(_src[1]* 255.0f + 0.5f),
                  (int)(_src[2]* 255.0f + 0.5f) );
  }
};

template <>
struct color_caster<Vec4i,Vec4f>
{
  typedef Vec4i return_type;

  inline static return_type cast(const Vec4f& _src)
  {
    return Vec4i( (int)(_src[0]* 255.0f + 0.5f),
                  (int)(_src[1]* 255.0f + 0.5f),
                  (int)(_src[2]* 255.0f + 0.5f),
                  (int)(_src[3]* 255.0f + 0.5f) );
  }
};

template <>
struct color_caster<Vec3ui,Vec3f>
{
  typedef Vec3ui return_type;

  inline static return_type cast(const Vec3f& _src)
  {
    return Vec3ui( (unsigned int)(_src[0]* 255.0f + 0.5f),
                   (unsigned int)(_src[1]* 255.0f + 0.5f),
                   (unsigned int)(_src[2]* 255.0f + 0.5f) );
  }
};

template <>
struct color_caster<Vec3ui,Vec4f>
{
  typedef Vec3ui return_type;

  inline static return_type cast(const Vec4f& _src)
  {
    return Vec3ui( (unsigned int)(_src[0]* 255.0f + 0.5f),
                   (unsigned int)(_src[1]* 255.0f + 0.5f),
                   (unsigned int)(_src[2]* 255.0f + 0.5f) );
  }
};

template <>
struct color_caster<Vec4ui,Vec4f>
{
  typedef Vec4ui return_type;

  inline static return_type cast(const Vec4f& _src)
  {
    return Vec4ui( (unsigned int)(_src[0]* 255.0f + 0.5f),
                   (unsigned int)(_src[1]* 255.0f + 0.5f),
                   (unsigned int)(_src[2]* 255.0f + 0.5f),
                   (unsigned int)(_src[3]* 255.0f + 0.5f) );
  }
};

template <>
struct color_caster<Vec4uc,Vec3f>
{
  typedef Vec4uc return_type;

  inline static return_type cast(const Vec3f& _src)
  {
    return Vec4uc( (unsigned char)(_src[0]* 255.0f + 0.5f),
                   (unsigned char)(_src[1]* 255.0f + 0.5f),
                   (unsigned char)(_src[2]* 255.0f + 0.5f),
                   (unsigned char)(255) );
  }
};

template <>
struct color_caster<Vec4f,Vec3f>
{
  typedef Vec4f return_type;

  inline static return_type cast(const Vec3f& _src)
  {
    return Vec4f( _src[0],
                  _src[1],
                  _src[2],
                  1.0f );
  }
};

template <>
struct color_caster<Vec4uc,Vec4f>
{
  typedef Vec4uc return_type;

  inline static return_type cast(const Vec4f& _src)
  {
    return Vec4uc( (unsigned char)(_src[0]* 255.0f + 0.5f),
                   (unsigned char)(_src[1]* 255.0f + 0.5f),
                   (unsigned char)(_src[2]* 255.0f + 0.5f),
                   (unsigned char)(_src[3]* 255.0f + 0.5f) );
  }
};

template <>
struct color_caster<Vec4f,Vec4i>
{
  typedef Vec4f return_type;

  inline static return_type cast(const Vec4i& _src)
  {
    const float f = 1.0f / 255.0f;
    return Vec4f( _src[0] * f, _src[1] *  f, _src[2] * f , _src[3] * f  );
  }
};

template <>
struct color_caster<Vec4uc,Vec3uc>
{
  typedef Vec4uc return_type;

  inline static return_type cast(const Vec3uc& _src)
  {
    return Vec4uc( _src[0], _src[1], _src[2], 255 );
  }
};

template <>
struct color_caster<Vec3f, Vec3uc>
{
  typedef Vec3f return_type;

  inline static return_type cast(const Vec3uc& _src)
  {
    const float f = 1.0f / 255.0f;
    return Vec3f(_src[0] * f, _src[1] *  f, _src[2] * f );
  }
};

template <>
struct color_caster<Vec3f, Vec4uc>
{
  typedef Vec3f return_type;

  inline static return_type cast(const Vec4uc& _src)
  {
    const float f = 1.0f / 255.0f;
    return Vec3f(_src[0] * f, _src[1] *  f, _src[2] * f );
  }
};

template <>
struct color_caster<Vec4f, Vec3uc>
{
  typedef Vec4f return_type;

  inline static return_type cast(const Vec3uc& _src)
  {
    const float f = 1.0f / 255.0f;
    return Vec4f(_src[0] * f, _src[1] *  f, _src[2] * f, 1.0f );
  }
};

template <>
struct color_caster<Vec4f, Vec4uc>
{
  typedef Vec4f return_type;

  inline static return_type cast(const Vec4uc& _src)
  {
    const float f = 1.0f / 255.0f;
    return Vec4f(_src[0] * f, _src[1] *  f, _src[2] * f, _src[3] * f );
  }
};

// ----------------------------------------------------------------------------


#ifndef DOXY_IGNORE_THIS

#if !defined(OM_CC_MSVC)
template <typename dst_t>
struct color_caster<dst_t,dst_t>
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


template <typename dst_t, typename src_t>
inline
typename color_caster<dst_t, src_t>::return_type
color_cast(const src_t& _src )
{
  return color_caster<dst_t, src_t>::cast(_src);
}

#endif
//-----------------------------------------------------------------------------

//@}


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_COLOR_CAST_HH defined
//=============================================================================


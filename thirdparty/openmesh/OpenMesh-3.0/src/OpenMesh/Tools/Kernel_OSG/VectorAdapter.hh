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

// ----------------------------------------------------------------------------

#ifndef OPENMESH_KERNEL_OSG_VECTORADAPTER_HH
#define OPENMESH_KERNEL_OSG_VECTORADAPTER_HH


//== INCLUDES =================================================================

#include <osg/Geometry>
#include <OpenMesh/Core/Utils/vector_cast.hh>

//== NAMESPACES ===============================================================

namespace OpenMesh {

//== CLASS DEFINITION =========================================================

// ----------------------------------------------------------------- class ----

#define OSG_VECTOR_TRAITS( VecType ) \
  template <> struct vector_traits< VecType > { \
    typedef VecType                vector_type; \
    typedef vector_type::ValueType value_type;  \
    typedef GenProg::Int2Type< vector_type::_iSize > typed_size; \
    \
    static const size_t size_ = vector_type::_iSize; \
    static size_t size() { return size_; } \
  }

/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Pnt4f );
/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Pnt3f );
/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Pnt2f );

/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Vec4f );
/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Vec3f );
/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Vec2f );

/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Pnt4d );
/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Pnt3d );
/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Pnt2d );

/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Vec4d );
/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Vec3d );

/// Vector traits for OpenSG vector type
OSG_VECTOR_TRAITS( osg::Vec4ub );


// ----------------------------------------------------------------------------


#define OSG_COLOR_TRAITS( VecType, N ) \
  template <> struct vector_traits< VecType > { \
    typedef VecType                vector_type; \
    typedef vector_type::ValueType value_type;  \
    typedef GenProg::Int2Type< N > typed_size; \
    \
    static const size_t size_ = N; \
    static size_t size() { return size_; } \
  }


/// Vector traits for OpenSG color type
OSG_COLOR_TRAITS( osg::Color3ub, 3 );
/// Vector traits for OpenSG color type
OSG_COLOR_TRAITS( osg::Color4ub, 4 );
/// Vector traits for OpenSG color type
OSG_COLOR_TRAITS( osg::Color3f,  3 );
/// Vector traits for OpenSG color type
OSG_COLOR_TRAITS( osg::Color4f,  4 );

#undef OSG_VECTOR_TRAITS


// ----------------------------------------
#if 1
#define PNT2VEC_CASTER( DST, SRC ) \
  template <> struct vector_caster< DST, SRC > { \
    typedef DST   dst_t; \
    typedef SRC   src_t; \
    typedef const dst_t& return_type; \
    inline static return_type cast( const src_t& _src ) {\
      return _src.subZero(); \
    } \
  }

/// convert Pnt3f to Vec3f
PNT2VEC_CASTER( osg::Vec3f, osg::Pnt3f );

/// convert Pnt4f to Vec4f
PNT2VEC_CASTER( osg::Vec4f, osg::Pnt4f );

/// convert Pnt3d to Vec3d
PNT2VEC_CASTER( osg::Vec3d, osg::Pnt3d );

/// convert Pnt4d to Vec4d
PNT2VEC_CASTER( osg::Vec4d, osg::Pnt4d );

#undef PNT2VEC
#else
 
  template <> 
  struct vector_caster< osg::Vec3f, osg::Pnt3f > 
  {
    typedef osg::Vec3f   dst_t;
    typedef osg::Pnt3f   src_t;

    typedef const dst_t& return_type;
    inline static return_type cast( const src_t& _src ) 
    {
      std::cout << "casting Pnt3f to Vec3f\n";
      return _src.subZero();
    }
  };

#endif
// ----------------------------------------

//@{
/// Adapter for osg vector member computing a scalar product
inline
osg::Vec3f::ValueType dot( const osg::Vec3f &_v1, const osg::Vec3f &_v2 )
{ return _v1.dot(_v2); }


inline
osg::Vec3f::ValueType dot( const osg::Vec3f &_v1, const osg::Pnt3f &_v2 )
{ return _v1.dot(_v2); }


inline
osg::Vec2f::ValueType dot( const osg::Vec2f &_v1, const osg::Vec2f &_v2 )
{ return _v1.dot(_v2); }


inline
osg::Vec3f cross( const osg::Vec3f &_v1, const osg::Vec3f &_v2 )
{ return _v1.cross(_v2); }
//@}

//=============================================================================
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_VECTORADAPTER_HH defined
//=============================================================================


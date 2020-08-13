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


/** \file Core/Mesh/Traits.hh
    This file defines the default traits and some convenience macros.
*/


//=============================================================================
//
//  CLASS Traits
//
//=============================================================================

#ifndef OPENMESH_TRAITS_HH
#define OPENMESH_TRAITS_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Core/Mesh/Handles.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {


//== CLASS DEFINITION =========================================================


/// Macro for defining the vertex attributes. See \ref mesh_type.
#define VertexAttributes(_i) enum { VertexAttributes = _i }

/// Macro for defining the halfedge attributes. See \ref mesh_type.
#define HalfedgeAttributes(_i) enum { HalfedgeAttributes = _i }

/// Macro for defining the edge attributes. See \ref mesh_type.
#define EdgeAttributes(_i) enum { EdgeAttributes = _i }

/// Macro for defining the face attributes. See \ref mesh_type.
#define FaceAttributes(_i) enum { FaceAttributes = _i }

/// Macro for defining the vertex traits. See \ref mesh_type.
#define VertexTraits \
  template <class Base, class Refs> struct VertexT : public Base

/// Macro for defining the halfedge traits. See \ref mesh_type.
#define HalfedgeTraits \
  template <class Base, class Refs> struct HalfedgeT : public Base

/// Macro for defining the edge traits. See \ref mesh_type.
#define EdgeTraits \
  template <class Base, class Refs> struct EdgeT : public Base

/// Macro for defining the face traits. See \ref mesh_type.
#define FaceTraits \
  template <class Base, class Refs> struct FaceT : public Base



//== CLASS DEFINITION =========================================================


/** \class DefaultTraits Traits.hh <OpenMesh/Mesh/Traits.hh>

    Base class for all traits.  All user traits should be derived from
    this class. You may enrich all basic items by additional
    properties or define one or more of the types \c Point, \c Normal,
    \c TexCoord, or \c Color.

    \see The Mesh docu section on \ref mesh_type.
    \see Traits.hh for a list of macros for traits classes.
*/
struct DefaultTraits
{
  /// The default coordinate type is OpenMesh::Vec3f.
  typedef Vec3f  Point;

  /// The default normal type is OpenMesh::Vec3f.
  typedef Vec3f  Normal;

  /// The default 1D texture coordinate type is float.
  typedef float  TexCoord1D;
  /// The default 2D texture coordinate type is OpenMesh::Vec2f.
  typedef Vec2f  TexCoord2D;
  /// The default 3D texture coordinate type is OpenMesh::Vec3f.
  typedef Vec3f  TexCoord3D;

  /// The default texture index type
  typedef int TextureIndex;

  /// The default color type is OpenMesh::Vec3uc.
  typedef Vec3uc Color;

#ifndef DOXY_IGNORE_THIS
  VertexTraits    {};
  HalfedgeTraits  {};
  EdgeTraits      {};
  FaceTraits      {};
#endif

  VertexAttributes(0);
  HalfedgeAttributes(Attributes::PrevHalfedge);
  EdgeAttributes(0);
  FaceAttributes(0);
};


//== CLASS DEFINITION =========================================================


/** Helper class to merge two mesh traits.
 *  \internal
 *
 *  With the help of this class it's possible to merge two mesh traits.
 *  Whereby \c _Traits1 overrides equally named symbols of \c _Traits2.
 *
 *  For your convenience use the provided defines \c OM_Merge_Traits
 *  and \c OM_Merge_Traits_In_Template instead.
 *
 *  \see OM_Merge_Traits, OM_Merge_Traits_In_Template
 */
template <class _Traits1, class _Traits2> struct MergeTraits
{
#ifndef DOXY_IGNORE_THIS
  struct Result
  {
    // Mipspro needs this (strange) typedef
    typedef _Traits1  T1;
    typedef _Traits2  T2;


    VertexAttributes   ( T1::VertexAttributes   | T2::VertexAttributes   );
    HalfedgeAttributes ( T1::HalfedgeAttributes | T2::HalfedgeAttributes );
    EdgeAttributes     ( T1::EdgeAttributes     | T2::EdgeAttributes     );
    FaceAttributes     ( T1::FaceAttributes     | T2::FaceAttributes     );


    typedef typename T1::Point    Point;
    typedef typename T1::Normal   Normal;
    typedef typename T1::Color    Color;
    typedef typename T1::TexCoord TexCoord;

    template <class Base, class Refs> class VertexT :
      public T1::template VertexT<
      typename T2::template VertexT<Base, Refs>, Refs>
    {};

    template <class Base, class Refs> class HalfedgeT :
      public T1::template HalfedgeT<
      typename T2::template HalfedgeT<Base, Refs>, Refs>
    {};


    template <class Base, class Refs> class EdgeT :
      public T1::template EdgeT<
      typename T2::template EdgeT<Base, Refs>, Refs>
    {};


    template <class Base, class Refs> class FaceT :
      public T1::template FaceT<
      typename T2::template FaceT<Base, Refs>, Refs>
    {};
  };
#endif
};


/**
    Macro for merging two traits classes _S1 and _S2 into one traits class _D.
    Note that in case of ambiguities class _S1 overrides _S2, especially
    the point/normal/color/texcoord type to be used is taken from _S1::Point /
    _S1::Normal / _S1::Color / _S1::TexCoord
*/
#define OM_Merge_Traits(_S1, _S2, _D) \
  typedef OpenMesh::MergeTraits<_S1, _S2>::Result _D;


/**
    Macro for merging two traits classes _S1 and _S2 into one traits class _D.
    Same as OM_Merge_Traits, but this can be used inside template classes.
*/
#define OM_Merge_Traits_In_Template(_S1, _S2, _D) \
  typedef typename OpenMesh::MergeTraits<_S1, _S2>::Result _D;


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_TRAITS_HH defined
//=============================================================================


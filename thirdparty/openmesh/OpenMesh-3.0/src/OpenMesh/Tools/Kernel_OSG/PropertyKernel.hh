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

#ifndef OPENMESH_KERNEL_OSG_PROPERTYKERNEL_HH
#define OPENMESH_KENREL_OSG_PROPERTYKERNEL_HH


//== INCLUDES =================================================================

#include <OpenMesh/Core/Utils/Property.hh>
#include <OpenMesh/Core/Mesh/BaseKernel.hh>
// --------------------
#include <OpenMesh/Tools/Kernel_OSG/PropertyT.hh>

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace Kernel_OSG {

//== CLASS DEFINITION =========================================================

/** Helper class, extending functionaliy of OpenMesh::BaseKernel to OpenSG
 *  specific property adaptors.
 *  \internal
 *  \todo Follow coding convention and rename class to PropertyKernelT
 */
template < typename IsTriMesh >
class PropertyKernel : public OpenMesh::BaseKernel
{
public:

  // --------------------------------------------------------------- item types

  typedef FPropHandleT<osg::UInt8>      FPTypesHandle;
  typedef FPropHandleT<osg::UInt32>     FPLengthsHandle;
  typedef FPropHandleT<osg::UInt32>     FIndicesHandle;

  typedef FP::GeoPTypesUI8              GeoPTypes;
  typedef FP::GeoPLengthsUI32           GeoPLengths;
  typedef FP::GeoIndicesUI32<IsTriMesh> GeoIndices;

  // ------------------------------------------------- constructor / destructor

  PropertyKernel() {}
  virtual ~PropertyKernel() { }


protected: // ---------------------------------------------- add osg properties

  // -------------------- vertex properties

  template < typename T >
  VPropHandleT<T> add_vpositions( const T& _t, const std::string& _n )
  { return VPropHandleT<T>(_add_vprop( new typename _t2vp<T>::prop(_n))); }

  template < typename T >
  VPropHandleT<T> add_vnormals( const T& _t, const std::string& _n )
  { return VPropHandleT<T>(_add_vprop( new typename _t2vn<T>::prop(_n) )); }

  template < typename T >
  VPropHandleT<T> add_vcolors( const T& _t, const std::string& _n )
  { return VPropHandleT<T>(_add_vprop( new typename _t2vc<T>::prop(_n) )); }

  template < typename T >
  VPropHandleT<T> add_vtexcoords( const T& _t, const std::string& _n )
  { return VPropHandleT<T>(_add_vprop( new typename _t2vtc<T>::prop(_n) )); }


  // -------------------- face properties

  FPTypesHandle add_fptypes( )
  { return FPTypesHandle(_add_fprop(new GeoPTypes)); }

  FPLengthsHandle add_fplengths( )
  { return FPLengthsHandle(_add_fprop(new GeoPLengths)); }

  FIndicesHandle add_findices( FPTypesHandle _pht, FPLengthsHandle _phl )
  { 
    GeoIndices *bp = new GeoIndices( fptypes(_pht), fplengths(_phl ) );
    return FIndicesHandle(_add_fprop( bp ) ); 
  }

protected: // ------------------------------------------- access osg properties
  
  template < typename T >
  typename _t2vp<T>::prop& vpositions( VPropHandleT<T> _ph ) 
  { return static_cast<typename _t2vp<T>::prop&>( _vprop( _ph ) ); }

  template < typename T >
  const typename _t2vp<T>::prop& vpositions( VPropHandleT<T> _ph) const
  { return static_cast<const typename _t2vp<T>::prop&>( _vprop( _ph ) ); }


  template < typename T >
  typename _t2vn<T>::prop& vnormals( VPropHandleT<T> _ph ) 
  { return static_cast<typename _t2vn<T>::prop&>( _vprop( _ph ) ); }

  template < typename T >
  const typename _t2vn<T>::prop& vnormals( VPropHandleT<T> _ph) const
  { return static_cast<const typename _t2vn<T>::prop&>( _vprop( _ph ) ); }


  template < typename T >
  typename _t2vc<T>::prop& vcolors( VPropHandleT<T> _ph )
  { return static_cast<typename _t2vc<T>::prop&>( _vprop( _ph ) ); }

  template < typename T >
  const typename _t2vc<T>::prop& vcolors( VPropHandleT<T> _ph ) const
  { return static_cast<const typename _t2vc<T>::prop&>( _vprop( _ph ) ); }


  template < typename T >
  typename _t2vtc<T>::prop& vtexcoords( VPropHandleT<T> _ph )
  { return static_cast<typename _t2vtc<T>::prop&>( _vprop( _ph ) ); }

  template < typename T >
  const typename _t2vtc<T>::prop& vtexcoords( VPropHandleT<T> _ph ) const
  { return static_cast<const typename _t2vtc<T>::prop&>( _vprop( _ph ) ); }


  //
  GeoPTypes& fptypes( FPTypesHandle _ph )
  { return static_cast<GeoPTypes&>( _fprop(_ph) ); }

  const GeoPTypes& fptypes( FPTypesHandle _ph ) const
  { return static_cast<const GeoPTypes&>( _fprop(_ph) ); }


  GeoPLengths& fplengths( FPLengthsHandle _ph )
  { return static_cast<GeoPLengths&>( _fprop(_ph) ); }

  const GeoPLengths& fplengths( FPLengthsHandle _ph ) const
  { return static_cast<const GeoPLengths&>( _fprop(_ph) ); }


  GeoIndices& findices( FIndicesHandle _ph )
  { return static_cast<GeoIndices&>( _fprop(_ph) ); }

  const GeoIndices& findices( FIndicesHandle _ph ) const
  { return static_cast<const GeoIndices&>( _fprop(_ph) ); }

    
protected: // ------------------------------------ access osg property elements

  template <typename T> 
  T& vpositions( VPropHandleT<T> _ph, VertexHandle _vh ) 
  { return vpositions(_ph)[_vh.idx()]; }

  template <class T>
  const T& vpositions( VPropHandleT<T> _ph, VertexHandle _vh ) const 
  { return vpositions(_ph)[_vh.idx()]; }


  template < typename T> 
  T& vnormals( VPropHandleT<T> _ph, VertexHandle _vh ) 
  { return vnormals(_ph)[_vh.idx()]; }

  template <class T>
  const T& vnormals( VPropHandleT<T> _ph, VertexHandle _vh ) const 
  { return vnormals(_ph)[_vh.idx()]; }


  template < typename T> 
  T& vcolors( VPropHandleT<T> _ph, VertexHandle _vh ) 
  { return vcolors(_ph)[_vh.idx()]; }

  template <class T>
  const T& vcolors( VPropHandleT<T> _ph, VertexHandle _vh ) const 
  { return vcolors(_ph)[_vh.idx()]; }


  template < typename T> 
  T& vtexcoords( VPropHandleT<T> _ph, VertexHandle _vh ) 
  { return vtexcoords(_ph)[_vh.idx()]; }

  template <class T>
  const T& vtexcoords( VPropHandleT<T> _ph, VertexHandle _vh ) const 
  { return vtexcoords(_ph)[_vh.idx()]; }


  // -------------------- access face property elements

  FPTypesHandle::value_type& 
  fptypes( FPTypesHandle _ph, FaceHandle _fh )
  { return fptypes( _ph )[ _fh.idx()]; }

  const FPTypesHandle::value_type& 
  fptypes( FPTypesHandle _ph, FaceHandle _fh ) const
  { return fptypes( _ph )[ _fh.idx()]; }


  FPLengthsHandle::value_type& 
  fplengths( FPLengthsHandle _ph, FaceHandle _fh )
  { return fplengths( _ph )[ _fh.idx()]; }

  const FPLengthsHandle::value_type& 
  fplengths( FPLengthsHandle _ph, FaceHandle _fh ) const
  { return fplengths( _ph )[ _fh.idx()]; }


  FIndicesHandle::value_type& 
  findices( FIndicesHandle _ph, FaceHandle _fh )
  { return findices( _ph )[ _fh.idx()]; }

  const FIndicesHandle::value_type& 
  findices( FIndicesHandle _ph, FaceHandle _fh ) const
  { return findices( _ph )[ _fh.idx()]; }

public:

  void stats(void)
  {
    std::cout << "#V : "  << n_vertices() << std::endl;
    std::cout << "#E : "  << n_edges() << std::endl;
    std::cout << "#F : "  << n_faces() << std::endl;
    property_stats();
  }
};


//=============================================================================
} // namespace Kernel_OSG
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_KERNEL_OSG_PROPERTYKERNEL_HH defined
//=============================================================================


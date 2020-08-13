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
//  CLASS OSGArrayKernelT
//
//=============================================================================


#ifndef OPENMESH_KERNELOSG_ARRAY_KERNEL_HH
#define OPENMEHS_KERNELOSG_ARRAY_KERNEL_HH


//== INCLUDES =================================================================

#include <vector>
// --------------------
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/GenProg.hh>
#include <OpenMesh/Core/Mesh/ArrayKernel.hh>
// --------------------
#include <OpenMesh/Tools/Kernel_OSG/AttribKernelT.hh>



//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace Kernel_OSG {

//== CLASS DEFINITION =========================================================


/** \ingroup mesh_kernels_group
 *   
 *  Mesh kernel using arrays for mesh item storage.
 *
 *  This mesh kernel uses the OpenSG GeoProperties as container
 *  to store the mesh items.
 *
 *  \note You do not have to use this class directly, use the predefined
 *  mesh-kernel combinations in \ref mesh_types_group.
 */
//     \see OpenMesh::ArrayHandleT
//     \see \ref mesh_type 


template <class AttribKernel, class FinalMeshItems>
class ArrayKernelT 
  : public OpenMesh::ArrayKernelT<AttribKernel, FinalMeshItems>
{
public:
  
  typedef ArrayKernelT<AttribKernel, FinalMeshItems>           This;
  typedef OpenMesh::ArrayKernelT<AttribKernel, FinalMeshItems> Base;

  // attributes
//   typedef typename Base::HasVertexNormals             HasVertexNormals;
//   typedef typename Base::HasVertexColors              HasVertexColors;
//   typedef typename Base::HasVertexTexCoords           HasVertexTexCoords;
//   typedef typename Base::HasVertexStatus              HasVertexStatus;
  typedef typename Base::HasPrevHalfedge              HasPrevHalfedge;
//   typedef typename Base::HasEdgeStatus                HasEdgeStatus;
//   typedef typename Base::HasFaceNormals               HasFaceNormals;
//   typedef typename Base::HasFaceColors                HasFaceColors;
//   typedef typename Base::HasFaceStatus                HasFaceStatus;

  // item types
  typedef typename FinalMeshItems::Vertex             Vertex;
  typedef typename FinalMeshItems::Halfedge           Halfedge;
  typedef typename FinalMeshItems::Edge               Edge;
  typedef typename FinalMeshItems::Face               Face;
  typedef typename FinalMeshItems::Point              Point;
  typedef typename FinalMeshItems::Normal             Normal;
  typedef typename FinalMeshItems::Color              Color;
  typedef typename FinalMeshItems::TexCoord           TexCoord;
  typedef typename FinalMeshItems::Scalar             Scalar;

//   // handles
//   typedef typename OpenMesh::VertexHandle       VertexHandle;    
//   typedef typename FinalMeshItems::HalfedgeHandle     HalfedgeHandle;  
//   typedef typename FinalMeshItems::EdgeHandle         EdgeHandle;      
//   typedef typename FinalMeshItems::FaceHandle         FaceHandle;      

  // iterators
  typedef std::vector<Vertex>                         VertexContainer;
  typedef std::vector<Edge>                           EdgeContainer;
  typedef std::vector<Face>                           FaceContainer;
  typedef typename VertexContainer::iterator          KernelVertexIter;
  typedef typename VertexContainer::const_iterator    KernelConstVertexIter;
  typedef typename EdgeContainer::iterator            KernelEdgeIter;
  typedef typename EdgeContainer::const_iterator      KernelConstEdgeIter;
  typedef typename FaceContainer::iterator            KernelFaceIter;
  typedef typename FaceContainer::const_iterator      KernelConstFaceIter;

public:

  ArrayKernelT() : Base()
  { }

  virtual ~ArrayKernelT()
  { }

public: // replacements

  void set_halfedge_handle(VertexHandle _vh, HalfedgeHandle _heh) { 
    Base::set_halfedge_handle( _vh, _heh );
  }

  void set_halfedge_handle(FaceHandle _fh, HalfedgeHandle _heh) { 
    Base::set_halfedge_handle( _fh, _heh );
    osg_sync( _fh );
  }

  void set_next_halfedge_handle(HalfedgeHandle _heh, HalfedgeHandle _nheh) { 
    Base::set_next_halfedge_handle( _heh, _nheh );
    osg_sync( face_handle( _heh ) ); // ##Changed
  }

  void garbage_collection(bool _v=true, bool _e=true, bool _f=true);

protected:  
  
  bool osg_sync( FaceHandle _fh )
  {    
    return _fh.is_valid() 
      ? osg_sync( _fh, typename Face::IsTriangle() ) 
      : false;
  }
  
private:

  bool osg_sync( FaceHandle _fh,  GenProg::Bool2Type<true> )
  {    
    HalfedgeHandle hh( halfedge_handle(_fh) );  
    if ( !hh.is_valid() ) return false;
    FaceHandle f1( _fh.idx() * 3 );
    set_face_indices( f1, to_vertex_handle(hh).idx() );

    hh = next_halfedge_handle(hh);  
    if ( !hh.is_valid() ) return false;
    FaceHandle f2( f1.idx()+1 );
    set_face_indices( f2, to_vertex_handle(hh).idx() );

    hh = next_halfedge_handle(hh);  
    if ( !hh.is_valid() ) return false;
    FaceHandle f3( f1.idx()+2 );
    set_face_indices( f3, to_vertex_handle(hh).idx() );

    set_face_types  ( _fh, GL_TRIANGLES );
    set_face_lengths( _fh, 3 );  

    return true;
  }

  bool osg_sync( FaceHandle _fh,  GenProg::Bool2Type<false> )
  {
    return false;
  }

};


template <class AttribKernel, class FinalMeshItems>
void
ArrayKernelT<AttribKernel, FinalMeshItems>::
garbage_collection(bool _v, bool _e, bool _f)
{
  Base::garbage_collection(_v, _e, _f);
  for (size_t fidx=0; fidx < n_faces(); ++fidx)
    osg_sync( FaceHandle(fidx) );
}

//=============================================================================
} // namespace Kernel_OSG
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_ARRAY_KERNEL_HH defined
//=============================================================================

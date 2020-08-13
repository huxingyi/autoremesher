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

#ifndef OPENMESH_ARRAY_ITEMS_HH
#define OPENMESH_ARRAY_ITEMS_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/GenProg.hh>
#include <OpenMesh/Core/Mesh/Handles.hh>


//== NAMESPACES ===============================================================

namespace OpenMesh {


//== CLASS DEFINITION =========================================================


/// Definition of mesh items for use in the ArrayKernel
struct ArrayItems
{

  //------------------------------------------------------ internal vertex type

  /// The vertex item
  class Vertex
  {
    friend class ArrayKernel;
    HalfedgeHandle  halfedge_handle_;
  };


  //---------------------------------------------------- internal halfedge type

#ifndef DOXY_IGNORE_THIS
  class Halfedge_without_prev
  {
    friend class ArrayKernel;
    FaceHandle      face_handle_;
    VertexHandle    vertex_handle_;
    HalfedgeHandle  next_halfedge_handle_;
  };
#endif

#ifndef DOXY_IGNORE_THIS
  class Halfedge_with_prev : public Halfedge_without_prev
  {
    friend class ArrayKernel;
    HalfedgeHandle  prev_halfedge_handle_;
  };
#endif

  //TODO: should be selected with config.h define
  typedef Halfedge_with_prev                Halfedge;
  typedef GenProg::Bool2Type<true>          HasPrevHalfedge;

  //-------------------------------------------------------- internal edge type
#ifndef DOXY_IGNORE_THIS
  class Edge
  {
    friend class ArrayKernel;
    Halfedge  halfedges_[2];
  };
#endif

  //-------------------------------------------------------- internal face type
#ifndef DOXY_IGNORE_THIS
  class Face
  {
    friend class ArrayKernel;
    HalfedgeHandle  halfedge_handle_;
  };
};
#endif

//=============================================================================
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_ITEMS_HH defined
//=============================================================================

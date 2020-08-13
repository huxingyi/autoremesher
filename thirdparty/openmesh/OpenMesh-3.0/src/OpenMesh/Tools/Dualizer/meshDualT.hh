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

/*
  Compute the dual of a mesh:
  - each face of the original mesh is replaced by a vertex at the center of gravity of the vertices of the face
  - each vertex of the original mesh is replaced by a face containing the dual vertices of its primal adjacent faces

  Changelog:
    - 29 mar 2010: initial work

  Programmer: 
    Clement Courbet - clement.courbet@ecp.fr

  (c) Clement Courbet 2010
*/

#ifndef OPENMESH_MESH_DUAL_H
#define OPENMESH_MESH_DUAL_H

//== INCLUDES =================================================================

// -------------------- STL
#include <vector>
#if defined(OM_CC_MIPS)
#  include <math.h>
#else
#  include <cmath>
#endif

#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Utils/Property.hh>

//== FORWARDDECLARATIONS ======================================================

//== NAMESPACES ===============================================================


namespace OpenMesh   { 
namespace Util { 

//== Function DEFINITION =========================================================

/** \brief create a dual mesh
*
* This function takes a mesh and computes the dual mesh of it. Each face of the original mesh is replaced by a vertex at the center of gravity of the vertices of the face.
* Each vertex of the original mesh is replaced by a face containing the dual vertices of its primal adjacent faces.
*/
template <typename MeshTraits>
PolyMesh_ArrayKernelT<MeshTraits>* MeshDual (PolyMesh_ArrayKernelT<MeshTraits> &primal)
{
  PolyMesh_ArrayKernelT<MeshTraits>* dual = new PolyMesh_ArrayKernelT<MeshTraits>();

  //we will need to reference which vertex in the dual is attached to each face in the primal
  //and which face of the dual is attached to each vertex in the primal.

  FPropHandleT< typename PolyMesh_ArrayKernelT<MeshTraits>::VertexHandle > primalToDual;
  primal.add_property(primalToDual);

  //for each face in the primal mesh, add a vertex at the center of gravity of the face
  for(typename PolyMesh_ArrayKernelT<MeshTraits>::ConstFaceIter fit=primal.faces_begin(); fit!=primal.faces_end(); ++fit)
  {
      typename PolyMesh_ArrayKernelT<MeshTraits>::Point centerPoint(0,0,0);
      typename PolyMesh_ArrayKernelT<MeshTraits>::Scalar degree= 0.0;
      for(typename PolyMesh_ArrayKernelT<MeshTraits>::ConstFaceVertexIter vit=primal.cfv_iter(*fit); vit.is_valid(); ++vit, ++degree)
        centerPoint += primal.point(*vit);
        assert(degree!=0);
        centerPoint /= degree;
        primal.property(primalToDual, *fit) = dual->add_vertex(centerPoint);
  }

  //for each vertex in the primal, add a face in the dual
  std::vector< typename PolyMesh_ArrayKernelT<MeshTraits>::VertexHandle >  face_vhandles;
  for(typename PolyMesh_ArrayKernelT<MeshTraits>::ConstVertexIter vit=primal.vertices_begin(); vit!=primal.vertices_end(); ++vit)
  {
    if(!primal.is_boundary(*vit))
    {
      face_vhandles.clear();
      for(typename PolyMesh_ArrayKernelT<MeshTraits>::ConstVertexFaceIter fit=primal.cvf_iter(*vit); fit.is_valid(); ++fit)
        face_vhandles.push_back(primal.property(primalToDual, *fit));
      dual->add_face(face_vhandles);
    }
  }

  primal.remove_property(primalToDual);

  return dual;

}

//=============================================================================
} // namespace Util
} // namespace OpenMesh
//=============================================================================

//=============================================================================
#endif // OPENMESH_MESH_DUAL_H defined
//=============================================================================



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
//  CLASS TriMeshT - IMPLEMENTATION
//
//=============================================================================


#define OPENMESH_TRIMESH_C


//== INCLUDES =================================================================


#include <OpenMesh/Core/Mesh/TriMeshT.hh>
#include <OpenMesh/Core/System/omstream.hh>
#include <vector>


//== NAMESPACES ==============================================================


namespace OpenMesh {


//== IMPLEMENTATION ==========================================================

template <class Kernel>
typename TriMeshT<Kernel>::Normal
TriMeshT<Kernel>::
calc_face_normal(FaceHandle _fh) const
{
  assert(this->halfedge_handle(_fh).is_valid());
  ConstFaceVertexIter fv_it(this->cfv_iter(_fh));

  const Point& p0(this->point(*fv_it));  ++fv_it;
  const Point& p1(this->point(*fv_it));  ++fv_it;
  const Point& p2(this->point(*fv_it));

  return PolyMesh::calc_face_normal(p0, p1, p2);
}

//=============================================================================
} // namespace OpenMesh
//=============================================================================

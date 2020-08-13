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

#ifndef OPENMESH_CIRCULATORS_HH
#define OPENMESH_CIRCULATORS_HH

//=============================================================================
//
//  Vertex and Face circulators for PolyMesh/TriMesh
//
//=============================================================================



//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>
#include <cassert>


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace Iterators {


//== FORWARD DECLARATIONS =====================================================


template <class Mesh> class VertexVertexIterT;
template <class Mesh> class VertexIHalfedgeIterT;
template <class Mesh> class VertexOHalfedgeIterT;
template <class Mesh> class VertexEdgeIterT;
template <class Mesh> class VertexFaceIterT;

template <class Mesh> class ConstVertexVertexIterT;
template <class Mesh> class ConstVertexIHalfedgeIterT;
template <class Mesh> class ConstVertexOHalfedgeIterT;
template <class Mesh> class ConstVertexEdgeIterT;
template <class Mesh> class ConstVertexFaceIterT;

template <class Mesh> class FaceVertexIterT;
template <class Mesh> class FaceHalfedgeIterT;
template <class Mesh> class FaceEdgeIterT;
template <class Mesh> class FaceFaceIterT;

template <class Mesh> class ConstFaceVertexIterT;
template <class Mesh> class ConstFaceHalfedgeIterT;
template <class Mesh> class ConstFaceEdgeIterT;
template <class Mesh> class ConstFaceFaceIterT;




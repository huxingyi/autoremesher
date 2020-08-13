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

#ifndef OPENMESH_CASTS_HH
#define OPENMESH_CASTS_HH
//== INCLUDES =================================================================

#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

//== NAMESPACES ===============================================================
namespace OpenMesh 
{

template <class Traits>
inline TriMesh_ArrayKernelT<Traits>& TRIMESH_CAST(PolyMesh_ArrayKernelT<Traits>& _poly_mesh)
{ return reinterpret_cast< TriMesh_ArrayKernelT<Traits>& >(_poly_mesh); }

template <class Traits>
inline const TriMesh_ArrayKernelT<Traits>& TRIMESH_CAST(const PolyMesh_ArrayKernelT<Traits>& _poly_mesh)
{ return reinterpret_cast< const TriMesh_ArrayKernelT<Traits>& >(_poly_mesh); }

template <class Traits>
inline PolyMesh_ArrayKernelT<Traits>& POLYMESH_CAST(TriMesh_ArrayKernelT<Traits>& _tri_mesh)
{ return reinterpret_cast< PolyMesh_ArrayKernelT<Traits>& >(_tri_mesh); }

template <class Traits>
inline const PolyMesh_ArrayKernelT<Traits>& POLYMESH_CAST(const TriMesh_ArrayKernelT<Traits>& _tri_mesh)
{ return reinterpret_cast< const PolyMesh_ArrayKernelT<Traits>& >(_tri_mesh); }

};
#endif//OPENMESH_CASTS_HH

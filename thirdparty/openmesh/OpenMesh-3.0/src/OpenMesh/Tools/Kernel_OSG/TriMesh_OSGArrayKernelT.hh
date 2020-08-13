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
//  CLASS TriMesh_OSGArrayKernelT
//
//=============================================================================


#ifndef OPENMESH_KERNEL_OSG_TRIMESH_OSGARRAYKERNEL_HH
#define OPENMESH_KERNEL_OSG_TRIMESH_OSGARRAYKERNEL_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
// --------------------
#include <OpenMesh/Core/Mesh/TriMeshT.hh>
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Core/Mesh/ArrayKernel.hh>
//#include <OpenMesh/Core/Mesh/ArrayItems.hh>
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <OpenMesh/Core/Mesh/FinalMeshItemsT.hh>
// --------------------
#include <OpenMesh/Tools/Kernel_OSG/VectorAdapter.hh>
#include <OpenMesh/Tools/Kernel_OSG/Traits.hh>
#include <OpenMesh/Tools/Kernel_OSG/ArrayKernelT.hh>
// --------------------
#include <osg/Geometry>


//== NAMESPACES ===============================================================


namespace OpenMesh   {
namespace Kernel_OSG {

//== CLASS DEFINITION =========================================================


/// Helper class to create a TriMesh-type based on Kernel_OSG::ArrayKernelT
template <class Traits>
struct TriMesh_OSGArrayKernel_GeneratorT
{
  typedef FinalMeshItemsT<ArrayItems, Traits, true>  MeshItems;
  typedef AttribKernelT<MeshItems>                   AttribKernel;
  typedef ArrayKernelT<AttribKernel, MeshItems>      MeshKernel;
  typedef TriMeshT<MeshKernel>                       Mesh;
};



/** \ingroup mesh_types_group 
    Triangle mesh based on the Kernel_OSG::ArrayKernelT.
    \see OpenMesh::TriMeshT
    \see OpenMesh::ArrayKernelT
*/
template <class Traits = Kernel_OSG::Traits>  
class TriMesh_OSGArrayKernelT 
  : public TriMesh_OSGArrayKernel_GeneratorT<Traits>::Mesh 
{};


//=============================================================================
} // namespace Kernel_OSG
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_KERNEL_OSG_TRIMESH_OSGARRAYKERNEL_HH
//=============================================================================

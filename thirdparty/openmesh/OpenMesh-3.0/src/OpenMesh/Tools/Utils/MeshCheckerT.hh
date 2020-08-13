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


#ifndef OPENMESH_MESHCHECKER_HH
#define OPENMESH_MESHCHECKER_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/System/omstream.hh>
#include <OpenMesh/Core/Utils/GenProg.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <iostream>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace Utils {

//== CLASS DEFINITION =========================================================

	      
/** Check integrity of mesh.
 *
 *  This class provides several functions to check the integrity of a mesh.
 */
template <class Mesh>
class MeshCheckerT
{
public:
   
  /// constructor
  MeshCheckerT(const Mesh& _mesh) : mesh_(_mesh) {}
 
  /// destructor
  ~MeshCheckerT() {}


  /// what should be checked?
  enum CheckTargets
  {
    CHECK_EDGES     = 1,
    CHECK_VERTICES  = 2,
    CHECK_FACES     = 4,
    CHECK_ALL       = 255
  };

  
  /// check it, return true iff ok
  bool check( unsigned int _targets=CHECK_ALL,
	      std::ostream&  _os= omerr());


private:

  bool is_deleted(typename Mesh::VertexHandle _vh) 
  { return (mesh_.has_vertex_status() ? mesh_.status(_vh).deleted() : false); }

  bool is_deleted(typename Mesh::EdgeHandle _eh) 
  { return (mesh_.has_edge_status() ? mesh_.status(_eh).deleted() : false); }

  bool is_deleted(typename Mesh::FaceHandle _fh) 
  { return (mesh_.has_face_status() ? mesh_.status(_fh).deleted() : false); }


  // ref to mesh
  const Mesh&  mesh_;
};


//=============================================================================
} // namespace Utils
} // namespace OpenMesh
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_MESHCHECKER_C)
#define OPENMESH_MESHCHECKER_TEMPLATES
#include "MeshCheckerT.cc"
#endif
//=============================================================================
#endif // OPENMESH_MESHCHECKER_HH defined
//=============================================================================


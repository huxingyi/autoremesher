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
//  Class SoOpenMeshNode
//
//  This class defines an basic inventor node to display an OpenMesh
//
//=============================================================================


#ifndef OPENMESH_SOOPENMESHNODE_HH
#define OPENMESH_SOOPENMESHNODE_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/Utils/GenProg.hh>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoShape.h>

//== NAMESPACES ===============================================================


namespace OpenMesh {


//== CLASS DEFINITION =========================================================


template <class Mesh>
class SoOpenMeshNodeT : public SoShape
{

  SO_NODE_HEADER(SoOpenMeshNodeT<Mesh>);

    
public:

  static void initClass();
  SoOpenMeshNodeT(const Mesh* _mesh=0);
  void setMesh(const Mesh* mesh) { d_mesh = mesh; }


protected:

  virtual void GLRender(SoGLRenderAction *action);
  virtual void computeBBox(SoAction *action, SbBox3f &box, SbVec3f &center);
  virtual void generatePrimitives(SoAction *action);


private:

  virtual ~SoOpenMeshNodeT() {};


  // Draw faces as triangles / polygons
  void drawFaces(bool _send_normals) { 
    typedef typename Mesh::Face Face;
    drawFaces(_send_normals, typename Face::IsTriangle());
  }
  void drawFaces(bool _send_normals, OpenMesh::GenProg::Bool2Type<true>);
  void drawFaces(bool _send_normals, OpenMesh::GenProg::Bool2Type<false>);


  // Generate primitives
  void genPrimitives(SoAction* _action) { 
    typedef typename Mesh::Face Face;
    genPrimitives(_action, typename Face::IsTriangle());
  }
  void genPrimitives(SoAction* _action, OpenMesh::GenProg::Bool2Type<true>);
  void genPrimitives(SoAction* _action, OpenMesh::GenProg::Bool2Type<false>);


  const Mesh*  mesh_;
};


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#if defined(INCLUDE_TEMPLATES) && !defined(OPENMESH_SOOPENMESHNODE_CC)
#  define OPENMESH_SOOPENMESHMODE_TEMPLATES
#  include "SoOpenMeshNodeT.cc"
#endif
//=============================================================================
#endif // OPENMESH_SOOPENMESHNODE_HH
//=============================================================================

/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2015, RWTH-Aachen University                 *
 *           Department of Computer Graphics and Multimedia                  *
 *                          All rights reserved.                             *
 *                            www.openmesh.org                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * This file is part of OpenMesh.                                            *
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 *    this list of conditions and the following disclaimer.                  *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 *    contributors may be used to endorse or promote products derived from   *
 *    this software without specific prior written permission.               *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * ========================================================================= */



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
  explicit SoOpenMeshNodeT(const Mesh* _mesh=0);
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
#  include "SoOpenMeshNodeT_impl.hh"
#endif
//=============================================================================
#endif // OPENMESH_SOOPENMESHNODE_HH
//=============================================================================

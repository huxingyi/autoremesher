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
//  Class SoOpenMeshNodeT - implementation
//
//=============================================================================


#define OPENMESH_SOOPENMESHNODE_CC

//== INCLUDES =================================================================

#include <GL/gl.h>
#include <Inventor/SbBox.h>
#include <Inventor/SoPrimitiveVertex.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/misc/SoState.h>
#include <Inventor/elements/SoLightModelElement.h>
#include <Inventor/bundles/SoMaterialBundle.h>

#include "SoOpenMeshNodeT.hh"
// Attention must be included after SoOpenMeshNodeT.hh
// as it redefines several macros!
#include "SoOpenMeshSupport.hh"

#include <OpenMesh/Core/Math/VectorT.hh>


//== NAMESPACES ============================================================== 


namespace OpenMesh {


//== IMPLEMENTATION ========================================================== 


// Helper functions: draw vertices
inline void glVertex(const OpenMesh::Vec3f& _v)  { glVertex3fv(_v); }
inline void glVertex(const OpenMesh::Vec3d& _v)  { glVertex3dv(_v); }

// Helper functions: draw normal
inline void glNormal(const OpenMesh::Vec3f& _n)  { glNormal3fv(_n); }
inline void glNormal(const OpenMesh::Vec3d& _n)  { glNormal3dv(_n); }

// Helper function: convert Vec to SbVec3f
template <class Vec> inline SbVec3f sbvec3f(const Vec& _v) { 
  return SbVec3f(_v[0], _v[1], _v[2]); 
}


//-----------------------------------------------------------------------------


template <class Mesh>
void
SoOpenMeshNodeT<Mesh>::initClass()
{
  SO_NODE_INIT_CLASS(SoOpenMeshNodeT<Mesh>, SoShape, "Shape");
}


//-----------------------------------------------------------------------------


template <class Mesh>
SoOpenMeshNodeT<Mesh>::SoOpenMeshNodeT(const Mesh* _mesh) :
  mesh_(_mesh)
{
  SO_NODE_CONSTRUCTOR(SoOpenMeshNodeT<Mesh>);
}


//-----------------------------------------------------------------------------


template <class Mesh>
void
SoOpenMeshNodeT<Mesh>::GLRender(SoGLRenderAction *action)
{
  if (mesh_ && shouldGLRender(action))
  {
    SoState*  state = action->getState();
    SbBool send_normals = (SoLightModelElement::get(state) !=
			   SoLightModelElement::BASE_COLOR);

    SoMaterialBundle mb(action);
    mb.sendFirst();

    drawFaces(send_normals);
  }
}


//----------------------------------------------------------------------------


template<class Mesh>
void
SoOpenMeshNodeT<Mesh>::
drawFaces(bool _send_normals, OpenMesh::GenProg::Bool2Type<true>)
{
  typename Mesh::ConstFaceIter    f_it(mesh_->faces_begin()), 
                                  f_end(mesh_->faces_end());

  typename Mesh::ConstFaceVertexIter fv_it;


  if (_send_normals)
  {
    glBegin(GL_TRIANGLES);
    for (; f_it!=f_end; ++f_it)
    {
      glNormal(mesh_->normal(f_it));
      fv_it = mesh_->cfv_iter(f_it.handle());
      glVertex(mesh_->point(fv_it));
      ++fv_it;
      glVertex(mesh_->point(fv_it));
      ++fv_it;
      glVertex(mesh_->point(fv_it));
    }
    glEnd();
  }
  else 
  {
    glBegin(GL_TRIANGLES);
    for (; f_it!=f_end; ++f_it)
    {
      fv_it = mesh_->cfv_iter(f_it.handle());
      glVertex(mesh_->point(fv_it));
      ++fv_it;
      glVertex(mesh_->point(fv_it));
      ++fv_it;
      glVertex(mesh_->point(fv_it));
    }
    glEnd();
  }
}


//----------------------------------------------------------------------------


template<class Mesh>
void
SoOpenMeshNodeT<Mesh>::
drawFaces(bool _send_normals, OpenMesh::GenProg::Bool2Type<false>)
{
  typename Mesh::ConstFaceIter    f_it(mesh_->faces_begin()), 
                                  f_end(mesh_->faces_end());

  typename Mesh::ConstFaceVertexIter fv_it;


  if (_send_normals)
  {
    for (; f_it!=f_end; ++f_it)
    {
      glBegin(GL_POLYGON);
      glNormal(mesh_->normal(f_it));
      for (fv_it=mesh_->cfv_iter(f_it.handle()); fv_it.is_valid() ++fv_it)
	glVertex(mesh_->point(fv_it));
      glEnd();
    }
  }
  else
  {
    for (; f_it!=f_end; ++f_it)
    {
      glBegin(GL_POLYGON);
      for (fv_it=mesh_->cfv_iter(f_it.handle()); fv_it.is_valid() ++fv_it)
	glVertex(mesh_->point(fv_it));
      glEnd();
    }
  }
}


//-----------------------------------------------------------------------------


template <class Mesh>
void
SoOpenMeshNodeT<Mesh>::generatePrimitives(SoAction* _action)
{
  if (mesh_)
    genPrimitives(_action);
}


//----------------------------------------------------------------------------


template<class Mesh>
void
SoOpenMeshNodeT<Mesh>::
genPrimitives(SoAction* _action, OpenMesh::GenProg::Bool2Type<true>)
{
  typename Mesh::ConstFaceIter    f_it(mesh_->faces_begin()), 
                                  f_end(mesh_->faces_end());

  typename Mesh::ConstFaceVertexIter fv_it;


  SoPrimitiveVertex pv;

  beginShape(_action, TRIANGLES);

  for (; f_it!=f_end; ++f_it)
  {
    pv.setNormal(sbvec3f(mesh_->normal(f_it)));
			 
    fv_it = mesh_->cfv_iter(f_it.handle());

    pv.setPoint(sbvec3f(mesh_->point(fv_it)));
    shapeVertex(&pv);
    ++fv_it;

    pv.setPoint(sbvec3f(mesh_->point(fv_it)));
    shapeVertex(&pv);
    ++fv_it;

    pv.setPoint(sbvec3f(mesh_->point(fv_it)));
    shapeVertex(&pv);
  }

  endShape();
}


//----------------------------------------------------------------------------


template<class Mesh>
void
SoOpenMeshNodeT<Mesh>::
genPrimitives(SoAction* _action, OpenMesh::GenProg::Bool2Type<false>)
{
  typename Mesh::ConstFaceIter    f_it(mesh_->faces_begin()), 
                                  f_end(mesh_->faces_end());

  typename Mesh::ConstFaceVertexIter fv_it;

  SoPrimitiveVertex pv;


  for (; f_it!=f_end; ++f_it)
  {
    beginShape(_action, POLYGON);

    pv.setNormal(sbvec3f(mesh_->normal(f_it)));

    for (fv_it=mesh_->cfv_iter(f_it.handle()); fv_it.is_valid() ++fv_it)
    {
      pv.setPoint(sbvec3f(mesh_->point(fv_it)));
      shapeVertex(&pv);
    }

    endShape();
  }
}


//-----------------------------------------------------------------------------


template <class Mesh>
void
SoOpenMeshNodeT<Mesh>::computeBBox(SoAction *action,
				   SbBox3f &box,
				   SbVec3f &center)
{
  if (mesh_ && mesh_->n_vertices())
  {
    typename Mesh::ConstVertexIter  vIt(mesh_->vertices_begin());
    typename Mesh::ConstVertexIter  vEnd(mesh_->vertices_end());

    typename Mesh::Point min(mesh_->point(vIt));
    typename Mesh::Point max(mesh_->point(vIt));
                         

    for (++vIt; vIt != vEnd; ++vIt)
    {
      max.maximize(mesh_->point(vIt));
      min.minimize(mesh_->point(vIt));
    }

    box.setBounds(SbVec3f(min[0],min[1],min[2]),
		  SbVec3f(max[0],max[1],max[2]));
  }
  
  else  box.setBounds(SbVec3f(0,0,0), SbVec3f(0,0,0));
}


//-----------------------------------------------------------------------------

#if 0
/* As we are using templates, we cannot use the predefined macro
   SO_NODE_SOURCE to automatically generate the following piece of
   code. Ugly, but necessary.  
*/

template <class Mesh>
SoType SoOpenMeshNodeT<Mesh>::classTypeId;

template <class Mesh>
SoFieldData *SoOpenMeshNodeT<Mesh>::fieldData;

template <class Mesh>
const SoFieldData **SoOpenMeshNodeT<Mesh>::parentFieldData;

template <class Mesh>
SbBool SoOpenMeshNodeT<Mesh>::firstInstance = TRUE;

template <class Mesh> 
SoType SoOpenMeshNodeT<Mesh>::getTypeId() const {
  return classTypeId; 
}

template <class Mesh> 
const SoFieldData* 
SoOpenMeshNodeT<Mesh>::getFieldData() const {
  SO__NODE_CHECK_CONSTRUCT("SoOpenMeshNodeT");
  return fieldData;
}

template <class Mesh> 
void* SoOpenMeshNodeT<Mesh>::createInstance() {
  return (void *)(new SoOpenMeshNodeT<Mesh>);
}

#else

SO_NODE_SOURCE(SoOpenMeshNodeT<Mesh>);

#endif

//=============================================================================
} // namespace OpenMesh
//=============================================================================

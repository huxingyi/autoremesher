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
//  Implements the baseclass for MeshWriter exporter modules
//
//=============================================================================


#ifndef __BASEEXPORTER_HH__
#define __BASEEXPORTER_HH__


//=== INCLUDES ================================================================


// STL
#include <vector>

// OpenMesh
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/Mesh/BaseKernel.hh>


//=== NAMESPACES ==============================================================


namespace OpenMesh {
namespace IO {


//=== EXPORTER ================================================================


/**
   Base class for exporter modules.
   The exporter modules provide an interface between the writer modules and
   the target data structure.
*/

class OPENMESHDLLEXPORT BaseExporter
{
public:

  virtual ~BaseExporter() { }


  // get vertex data
  virtual Vec3f  point(VertexHandle _vh)    const = 0;
  virtual Vec3f  normal(VertexHandle _vh)   const = 0;
  virtual Vec3uc color(VertexHandle _vh)    const = 0;
  virtual Vec4uc colorA(VertexHandle _vh)   const = 0;
  virtual Vec3ui colori(VertexHandle _vh)    const = 0;
  virtual Vec4ui colorAi(VertexHandle _vh)   const = 0;
  virtual Vec3f colorf(VertexHandle _vh)    const = 0;
  virtual Vec4f colorAf(VertexHandle _vh)   const = 0;
  virtual Vec2f  texcoord(VertexHandle _vh) const = 0;


  // get face data
  virtual unsigned int
  get_vhandles(FaceHandle _fh,
	       std::vector<VertexHandle>& _vhandles) const=0;
  virtual Vec3f  normal(FaceHandle _fh)      const = 0;
  virtual Vec3uc color (FaceHandle _fh)      const = 0;
  virtual Vec4uc colorA(FaceHandle _fh)      const = 0;
  virtual Vec3ui colori(FaceHandle _fh)    const = 0;
  virtual Vec4ui colorAi(FaceHandle _fh)   const = 0;
  virtual Vec3f colorf(FaceHandle _fh)    const = 0;
  virtual Vec4f colorAf(FaceHandle _fh)   const = 0;

  // get edge data
  virtual Vec3uc color(EdgeHandle _eh)    const = 0;
  virtual Vec4uc colorA(EdgeHandle _eh)   const = 0;
  virtual Vec3ui colori(EdgeHandle _eh)    const = 0;
  virtual Vec4ui colorAi(EdgeHandle _eh)   const = 0;
  virtual Vec3f colorf(EdgeHandle _eh)    const = 0;
  virtual Vec4f colorAf(EdgeHandle _eh)   const = 0;

  // get reference to base kernel
  virtual const BaseKernel* kernel() { return 0; }


  // query number of faces, vertices, normals, texcoords
  virtual size_t n_vertices()   const = 0;
  virtual size_t n_faces()      const = 0;
  virtual size_t n_edges()      const = 0;


  // property information
  virtual bool is_triangle_mesh()     const { return false; }
  virtual bool has_vertex_normals()   const { return false; }
  virtual bool has_vertex_colors()    const { return false; }
  virtual bool has_vertex_texcoords() const { return false; }
  virtual bool has_edge_colors()      const { return false; }
  virtual bool has_face_normals()     const { return false; }
  virtual bool has_face_colors()      const { return false; }
};


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================

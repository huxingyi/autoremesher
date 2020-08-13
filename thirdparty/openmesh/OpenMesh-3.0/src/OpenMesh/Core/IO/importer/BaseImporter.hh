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
//  Implements the baseclass for IOManager importer modules
//
//=============================================================================


#ifndef __BASEIMPORTER_HH__
#define __BASEIMPORTER_HH__


//=== INCLUDES ================================================================


// STL
#include <vector>

// OpenMesh
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/Mesh/BaseKernel.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace IO {


//=== IMPLEMENTATION ==========================================================


/**  Base class for importer modules. Importer modules provide an
 *   interface between the loader modules and the target data
 *   structure. This is basically a wrapper providing virtual versions
 *   for the required mesh functions.
 */
class OPENMESHDLLEXPORT BaseImporter
{
public:

  // base class needs virtual destructor
  virtual ~BaseImporter() {}


  // add a vertex with coordinate \c _point
  virtual VertexHandle add_vertex(const Vec3f& _point) = 0;

  // add a face with indices _indices refering to vertices
  typedef std::vector<VertexHandle> VHandles;
  virtual FaceHandle add_face(const VHandles& _indices) = 0;

  // add texture coordinates per face, _vh references the first texcoord
  virtual void add_face_texcoords( FaceHandle _fh, VertexHandle _vh, const std::vector<Vec2f>& _face_texcoords) = 0;

  // Set the texture index for a face
  virtual void set_face_texindex( FaceHandle _fh, int _texId ) = 0;

  // set vertex normal
  virtual void set_normal(VertexHandle _vh, const Vec3f& _normal) = 0;

  // set vertex color
  virtual void set_color(VertexHandle _vh, const Vec3uc& _color) = 0;

  // set vertex color
  virtual void set_color(VertexHandle _vh, const Vec4uc& _color) = 0;

  // set vertex color
  virtual void set_color(VertexHandle _vh, const Vec3f& _color) = 0;

  // set vertex color
  virtual void set_color(VertexHandle _vh, const Vec4f& _color) = 0;

  // set vertex texture coordinate
  virtual void set_texcoord(VertexHandle _vh, const Vec2f& _texcoord) = 0;

  // set vertex texture coordinate
  virtual void set_texcoord(HalfedgeHandle _heh, const Vec2f& _texcoord) = 0;

  // set edge color
  virtual void set_color(EdgeHandle _eh, const Vec3uc& _color) = 0;

  // set edge color
  virtual void set_color(EdgeHandle _eh, const Vec4uc& _color) = 0;

  // set edge color
  virtual void set_color(EdgeHandle _eh, const Vec3f& _color) = 0;

  // set edge color
  virtual void set_color(EdgeHandle _eh, const Vec4f& _color) = 0;

  // set face normal
  virtual void set_normal(FaceHandle _fh, const Vec3f& _normal) = 0;

  // set face color
  virtual void set_color(FaceHandle _fh, const Vec3uc& _color) = 0;

  // set face color
  virtual void set_color(FaceHandle _fh, const Vec4uc& _color) = 0;

  // set face color
  virtual void set_color(FaceHandle _fh, const Vec3f& _color) = 0;

  // set face color
  virtual void set_color(FaceHandle _fh, const Vec4f& _color) = 0;

  // Store a property in the mesh mapping from an int to a texture file
  // Use set_face_texindex to set the index for each face
  virtual void add_texture_information( int _id , std::string _name ) = 0;

  // get reference to base kernel
  virtual BaseKernel* kernel() { return 0; }

  virtual bool is_triangle_mesh()     const { return false; }

  // reserve mem for elements
  virtual void reserve( unsigned int /* nV */,
		                  unsigned int /* nE */,
		                  unsigned int /* nF */) {}

  // query number of faces, vertices, normals, texcoords
  virtual size_t n_vertices()   const = 0;
  virtual size_t n_faces()      const = 0;
  virtual size_t n_edges()      const = 0;


  // pre-processing
  virtual void prepare()  {}

  // post-processing
  virtual void finish()  {}
};


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================

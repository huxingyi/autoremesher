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
//  Implements an exporter module for arbitrary OpenMesh meshes
//
//=============================================================================


#ifndef __EXPORTERT_HH__
#define __EXPORTERT_HH__


//=== INCLUDES ================================================================

// C++
#include <vector>

// OpenMesh
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <OpenMesh/Core/Utils/GenProg.hh>
#include <OpenMesh/Core/Utils/vector_cast.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>
#include <OpenMesh/Core/IO/exporter/BaseExporter.hh>


//=== NAMESPACES ==============================================================

namespace OpenMesh {
namespace IO {


//=== EXPORTER CLASS ==========================================================

/**
 *  This class template provides an exporter module for OpenMesh meshes.
 */
template <class Mesh>
class ExporterT : public BaseExporter
{
public:

  // Constructor
  ExporterT(const Mesh& _mesh) : mesh_(_mesh) {}


  // get vertex data

  Vec3f  point(VertexHandle _vh)    const
  {
    return vector_cast<Vec3f>(mesh_.point(_vh));
  }

  Vec3f  normal(VertexHandle _vh)   const
  {
    return (mesh_.has_vertex_normals()
	    ? vector_cast<Vec3f>(mesh_.normal(_vh))
	    : Vec3f(0.0f, 0.0f, 0.0f));
  }

  Vec3uc color(VertexHandle _vh)    const
  {
    return (mesh_.has_vertex_colors()
	    ? color_cast<Vec3uc>(mesh_.color(_vh))
	    : Vec3uc(0, 0, 0));
  }

  Vec4uc colorA(VertexHandle _vh)   const
  {
    return (mesh_.has_vertex_colors()
      ? color_cast<Vec4uc>(mesh_.color(_vh))
      : Vec4uc(0, 0, 0, 0));
  }

  Vec3ui colori(VertexHandle _vh)    const
  {
    return (mesh_.has_vertex_colors()
	    ? color_cast<Vec3ui>(mesh_.color(_vh))
	    : Vec3ui(0, 0, 0));
  }

  Vec4ui colorAi(VertexHandle _vh)   const
  {
    return (mesh_.has_vertex_colors()
      ? color_cast<Vec4ui>(mesh_.color(_vh))
      : Vec4ui(0, 0, 0, 0));
  }

  Vec3f colorf(VertexHandle _vh)    const
  {
    return (mesh_.has_vertex_colors()
	    ? color_cast<Vec3f>(mesh_.color(_vh))
	    : Vec3f(0, 0, 0));
  }

  Vec4f colorAf(VertexHandle _vh)   const
  {
    return (mesh_.has_vertex_colors()
      ? color_cast<Vec4f>(mesh_.color(_vh))
      : Vec4f(0, 0, 0, 0));
  }

  Vec2f  texcoord(VertexHandle _vh) const
  {
#if defined(OM_CC_GCC) && (OM_CC_VERSION<30000)
    // Workaround!
    // gcc 2.95.3 exits with internal compiler error at the
    // code below!??? **)
    if (mesh_.has_vertex_texcoords2D())
      return vector_cast<Vec2f>(mesh_.texcoord2D(_vh));
    return Vec2f(0.0f, 0.0f);
#else // **)
    return (mesh_.has_vertex_texcoords2D()
	    ? vector_cast<Vec2f>(mesh_.texcoord2D(_vh))
	    : Vec2f(0.0f, 0.0f));
#endif
  }

  // get edge data

  Vec3uc color(EdgeHandle _eh)    const
  {
      return (mesh_.has_edge_colors()
      ? color_cast<Vec3uc>(mesh_.color(_eh))
      : Vec3uc(0, 0, 0));
  }

  Vec4uc colorA(EdgeHandle _eh)   const
  {
      return (mesh_.has_edge_colors()
      ? color_cast<Vec4uc>(mesh_.color(_eh))
      : Vec4uc(0, 0, 0, 0));
  }

  Vec3ui colori(EdgeHandle _eh)    const
  {
      return (mesh_.has_edge_colors()
      ? color_cast<Vec3ui>(mesh_.color(_eh))
      : Vec3ui(0, 0, 0));
  }

  Vec4ui colorAi(EdgeHandle _eh)   const
  {
      return (mesh_.has_edge_colors()
      ? color_cast<Vec4ui>(mesh_.color(_eh))
      : Vec4ui(0, 0, 0, 0));
  }

  Vec3f colorf(EdgeHandle _eh)    const
  {
    return (mesh_.has_vertex_colors()
	    ? color_cast<Vec3f>(mesh_.color(_eh))
	    : Vec3f(0, 0, 0));
  }

  Vec4f colorAf(EdgeHandle _eh)   const
  {
    return (mesh_.has_vertex_colors()
      ? color_cast<Vec4f>(mesh_.color(_eh))
      : Vec4f(0, 0, 0, 0));
  }

  // get face data

  unsigned int get_vhandles(FaceHandle _fh,
			    std::vector<VertexHandle>& _vhandles) const
  {
    unsigned int count(0);
    _vhandles.clear();
    for (typename Mesh::CFVIter fv_it=mesh_.cfv_iter(_fh); fv_it.is_valid(); ++fv_it)
    {
      _vhandles.push_back(*fv_it);
      ++count;
    }
    return count;
  }

  Vec3f  normal(FaceHandle _fh)   const
  {
    return (mesh_.has_face_normals()
            ? vector_cast<Vec3f>(mesh_.normal(_fh))
            : Vec3f(0.0f, 0.0f, 0.0f));
  }

  Vec3uc  color(FaceHandle _fh)   const
  {
    return (mesh_.has_face_colors()
            ? color_cast<Vec3uc>(mesh_.color(_fh))
            : Vec3uc(0, 0, 0));
  }

  Vec4uc  colorA(FaceHandle _fh)   const
  {
    return (mesh_.has_face_colors()
            ? color_cast<Vec4uc>(mesh_.color(_fh))
            : Vec4uc(0, 0, 0, 0));
  }

  Vec3ui  colori(FaceHandle _fh)   const
  {
    return (mesh_.has_face_colors()
            ? color_cast<Vec3ui>(mesh_.color(_fh))
            : Vec3ui(0, 0, 0));
  }

  Vec4ui  colorAi(FaceHandle _fh)   const
  {
    return (mesh_.has_face_colors()
            ? color_cast<Vec4ui>(mesh_.color(_fh))
            : Vec4ui(0, 0, 0, 0));
  }

  Vec3f colorf(FaceHandle _fh)    const
  {
    return (mesh_.has_vertex_colors()
	    ? color_cast<Vec3f>(mesh_.color(_fh))
	    : Vec3f(0, 0, 0));
  }

  Vec4f colorAf(FaceHandle _fh)   const
  {
    return (mesh_.has_vertex_colors()
      ? color_cast<Vec4f>(mesh_.color(_fh))
      : Vec4f(0, 0, 0, 0));
  }

  virtual const BaseKernel* kernel() { return &mesh_; }


  // query number of faces, vertices, normals, texcoords
  size_t n_vertices()  const { return mesh_.n_vertices(); }
  size_t n_faces()     const { return mesh_.n_faces(); }
  size_t n_edges()     const { return mesh_.n_edges(); }


  // property information
  bool is_triangle_mesh() const
  { return Mesh::is_triangles(); }

  bool has_vertex_normals()   const { return mesh_.has_vertex_normals();   }
  bool has_vertex_colors()    const { return mesh_.has_vertex_colors();    }
  bool has_vertex_texcoords() const { return mesh_.has_vertex_texcoords2D(); }
  bool has_edge_colors()      const { return mesh_.has_edge_colors();      }
  bool has_face_normals()     const { return mesh_.has_face_normals();     }
  bool has_face_colors()      const { return mesh_.has_face_colors();      }

private:

   const Mesh& mesh_;
};


//=============================================================================
} // namespace IO
} // namespace OpenMesh
//=============================================================================
#endif
//=============================================================================

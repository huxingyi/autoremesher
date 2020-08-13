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


/** \file bindT.hh

    Bind an OpenMesh to a OpenSG geometry node. Be aware that due to
    this link the geometry node maybe modified. For instance triangle
    strips are converted to regular triangles.
*/


//=============================================================================
//
//  CLASS Traits
//
//=============================================================================

#ifndef OPENMESH_KERNEL_OSG_BINDT_HH
#define OPENMESH_KERNEL_OSG_BINDT_HH


//== INCLUDES =================================================================


#include <functional>
#include <algorithm>
//
#include <OpenMesh/Core/Mesh/TriMeshT.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>
#include <OpenMesh/Tools/Utils/GLConstAsString.hh>
#include <OpenSG/OSGGeometry.h>
//
#include "color_cast.hh"

//== NAMESPACES ===============================================================

namespace OpenMesh  {
namespace Kernel_OSG {


//== CLASS DEFINITION =========================================================

inline
bool type_is_valid( unsigned char _t )
{
  return _t == GL_TRIANGLES
    ||   _t == GL_TRIANGLE_STRIP
    ||   _t == GL_QUADS
    ||   _t == GL_POLYGON;
}


/** Bind a OpenSG geometry to a mesh.
 *
 *  \param _mesh The mesh object to bind the geometry to.
 *  \param _geo  The geometry object to bind.
 *  \return      true if the connection has been established else false.
 */
template < typename Mesh > inline
bool bind( osg::GeometryPtr& _geo, Mesh& _mesh )
{
  _geo = _mesh.createGeometryPtr();
}

/** Bind a mesh object to geometry. The binder is able to handle
 *  non-indexed and indexed geometry. Multi-indexed geometry is not
 *  supported.
 *
 *  \param _mesh The mesh object to bind.
 *  \param _geo  The geometry object to bind to.
 *  \return      true if the connection has been established else false.
 */
template < typename Mesh > inline
bool bind( Mesh& _mesh, osg::GeometryPtr& _geo )
{
  using namespace OpenMesh;
  using namespace osg;
  using namespace std;

  bool ok = true;

  // pre-check if types are supported

  GeoPTypesPtr types = _geo->getTypes();

  if ( (size_t)count_if( types->getData(), types->getData()+types->size(),
                         ptr_fun(type_is_valid) ) != (size_t)types->size() )
    return false;

  // pre-check if it is a multi-indexed geometry, which is not supported!

  if ( _geo->getIndexMapping().getSize() > 1 )
  {
    omerr << "OpenMesh::Kernel_OSG::bind(): Multi-indexed geometry is not supported!\n";
    return false;
  }


  // create shortcuts

  GeoPLengthsPtr  lengths = _geo->getLengths();
  GeoIndicesPtr   indices = _geo->getIndices();
  GeoPositionsPtr pos     = _geo->getPositions();
  GeoNormalsPtr   normals = _geo->getNormals();
  GeoColorsPtr    colors  = _geo->getColors();
 
  
  // -------------------- now convert everything to polygon/triangles

  size_t tidx, bidx; // types; base index into indices
  vector< VertexHandle > vhandles;

  // ---------- initialize geometry

  {
    VertexHandle vh;
    typedef typename Mesh::Color color_t;

    bool bind_normal = (normals!=NullFC) && _mesh.has_vertex_normals();
    bool bind_color  = (colors !=NullFC) && _mesh.has_vertex_colors();

    for (bidx=0; bidx < pos->size(); ++bidx)
    {
      vh = _mesh.add_vertex( pos->getValue(bidx) );
      if ( bind_normal )
        _mesh.set_normal(vh, normals->getValue(bidx));
      if ( bind_color )
        _mesh.set_color(vh, color_cast<color_t>(colors->getValue(bidx)));
    }
  }

  // ---------- create topology

  FaceHandle   fh;

  size_t max_bidx = indices != NullFC ? indices->size() : pos->size();

  for (bidx=tidx=0; ok && tidx<types->size() && bidx < max_bidx; ++tidx)
  {
    switch( types->getValue(tidx) )
    {
      case GL_TRIANGLES:
        vhandles.resize(3);
        for(size_t lidx=0; lidx < lengths->getValue(tidx)-2; lidx+=3)
        {
          if (indices == NullFC ) {
            vhandles[0] = VertexHandle(bidx+lidx);
            vhandles[1] = VertexHandle(bidx+lidx+1);
            vhandles[2] = VertexHandle(bidx+lidx+2);
          }
          else {
            vhandles[0] = VertexHandle(indices->getValue(bidx+lidx  ) );
            vhandles[1] = VertexHandle(indices->getValue(bidx+lidx+1) );
            vhandles[2] = VertexHandle(indices->getValue(bidx+lidx+2) );
          }

          if ( !(fh = _mesh.add_face( vhandles )).is_valid() )
          {
            // if fh is complex try swapped order
            swap(vhandles[2], vhandles[1]);
            fh = _mesh.add_face( vhandles );
          }
          ok = fh.is_valid();
        }
        break;

      case GL_TRIANGLE_STRIP:
        vhandles.resize(3);
        for (size_t lidx=0; lidx < lengths->getValue(tidx)-2; ++lidx)
        {
          if (indices == NullFC ) {
            vhandles[0] = VertexHandle(bidx+lidx);
            vhandles[1] = VertexHandle(bidx+lidx+1);
            vhandles[2] = VertexHandle(bidx+lidx+2);
          }
          else {
            vhandles[0] = VertexHandle(indices->getValue(bidx+lidx  ) );
            vhandles[1] = VertexHandle(indices->getValue(bidx+lidx+1) );
            vhandles[2] = VertexHandle(indices->getValue(bidx+lidx+2) );
          }

          if (vhandles[0]!=vhandles[2] &&
              vhandles[0]!=vhandles[1] &&
              vhandles[1]!=vhandles[2])
          {
            // if fh is complex try swapped order
            bool swapped(false);

            if (lidx % 2) // odd numbered triplet must be reordered
              swap(vhandles[2], vhandles[1]);
              
            if ( !(fh = _mesh.add_face( vhandles )).is_valid() )
            {
              omlog << "OpenMesh::Kernel_OSG::bind(): complex entity!\n";

              swap(vhandles[2], vhandles[1]);
              fh = _mesh.add_face( vhandles );
              swapped = true;
            }
            ok = fh.is_valid();
          }
        }
        break;

      case GL_QUADS:
        vhandles.resize(4);
        for(size_t nf=_mesh.n_faces(), lidx=0; 
            lidx < lengths->getValue(tidx)-3; lidx+=4)
        {
          if (indices == NullFC ) {
            vhandles[0] = VertexHandle(bidx+lidx);
            vhandles[1] = VertexHandle(bidx+lidx+1);
            vhandles[2] = VertexHandle(bidx+lidx+2);
            vhandles[3] = VertexHandle(bidx+lidx+3);
          }
          else {
            vhandles[0] = VertexHandle(indices->getValue(bidx+lidx  ) );
            vhandles[1] = VertexHandle(indices->getValue(bidx+lidx+1) );
            vhandles[2] = VertexHandle(indices->getValue(bidx+lidx+2) );
            vhandles[3] = VertexHandle(indices->getValue(bidx+lidx+3) );
          }

          fh = _mesh.add_face( vhandles );
          ok = ( Mesh::Face::is_triangle() && (_mesh.n_faces()==(nf+2)))
            || fh.is_valid();
          nf = _mesh.n_faces();
        }
        break;

      case GL_POLYGON:
      {
        size_t ne = lengths->getValue(tidx);
        size_t nf = _mesh.n_faces();

        vhandles.resize(ne);

        for(size_t lidx=0; lidx < ne; ++lidx)
          vhandles[lidx] = (indices == NullFC)
            ? VertexHandle(bidx+lidx)
            : VertexHandle(indices->getValue(bidx+lidx) );

        fh = _mesh.add_face( vhandles );
        ok = ( Mesh::Face::is_triangle() && (_mesh.n_faces()==nf+ne-2) )
          || fh.is_valid();
        
        break;
      }
      default:
        cerr << "Warning! Skipping unsupported type " 
             << types->getValue(tidx) << " '"
             << Utils::GLenum_as_string( types->getValue(tidx) ) << "'\n";
    }

    // update base index into indices for next face type
    bidx += lengths->getValue(tidx);
  }

  if (ok)
    ok=_mesh.bind(_geo);
  else
    _mesh.clear();

  return ok;
}


//=============================================================================
} // namespace Kernel_OSG
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_KERNEL_OSG_BINDT_HH defined
//=============================================================================


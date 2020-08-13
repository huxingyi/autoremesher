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
//  Mesh Items Concept
//
//=============================================================================


#error this file is for documentation purposes only


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace Concepts { 


//== CLASS DEFINITION =========================================================
	      

/** \ingroup mesh_concepts_group
    The mesh items class encapsulates the types VertexT, HalfedgeT,
    EdgeT, and FaceT.
    \see VertexT, HalfedgeT, EdgeT, FaceT
*/
struct MeshItems {


/** Interface for the internal vertex type. This minimal interface
    must be provided by every vertex. It's up to the mesh kernel (or
    the items used by the mesh kernel) to implement it.

    All methods marked as internal should only be used by the mesh
    kernel.  
*/
template <class Refs_> class VertexT
{
public:

  /// Re-export the template argument Refs. This \b must be done!
  typedef Refs_  Refs;

  /// Default constructor
  VertexT();

  /// Get an outgoing halfedge 
  HalfedgeHandle halfedge_handle()  const;
  /// Set the outgoing halfedge link
  void set_halfedge_handle(HalfedgeHandle _eh);
};




/** Interface for the internal halfedge type. This minimal interface
    must be provided by every halfedge. It's up to the mesh kernel (or
    the items used by the mesh kernel) to implement it.

    All methods marked as internal should only be used by the mesh
    kernel.  
*/
template <class Refs_> class HalfedgeT
{
public:
    
  /// Re-export the template argument Refs. This \b must be done!
  typedef Refs_  Refs;

  /** Get the vertex the halfedge point to.
      \internal */
  VertexHandle vertex_handle() const;

  /** Set the vertex the halfedge point to.
      \internal */
  void set_vertex_handle(VertexHandle _vh);

  /** Get the face this halfedge belongs to.
      \internal */
  FaceHandle face_handle()  const;

  /** Set the face this halfedge belongs to.
      \internal */
  void set_face_handle(FaceHandle _fh);
    
  /** Get the next halfedge inside this face.
      \internal */
  HalfedgeHandle next_halfedge_handle()  const;

  /** Set the next halfedge inside this face.
      \internal */
  void set_next_halfedge_handle(HalfedgeHandle _eh);
};




/** Interface for the internal edge type. This minimal interface must
    be provided by every edge. It's up to the mesh kernel (or the
    items used by the mesh kernel) to implement it.

    All methods marked as internal should only be used by the mesh
    kernel.  
*/
template <class Refs_> class EdgeT
{
public:

  /// Re-export the template argument Refs. This \b must be done!
  typedef Refs_  Refs;

  /** Store two halfedges.
      \internal */
  Halfedge  halfedges[2];
};




/** Interface for the internal face type. This minimal interface must
    be provided by every face. It's up to the mesh kernel (or the
    items used by the mesh kernel) to implement it.

    All methods marked as internal should only be used by the mesh
    kernel.  
*/
template <class Refs_> class FaceT
{
public:

  /// Re-export the template argument Refs. This \b must be done!
  typedef Refs_  Refs;

  /** Compile-time-tag: is this face a triangle? Should be typedef'ed
      to either GenProg::TagTrue or GenProg::TagFalse */
  typedef GenProg::TagTrue   IsTriangle;
  /// Run-time test: is this face a triangle?
  static bool is_triangle();

  /// Get the number of vertices of this face.
  unsigned char n_vertices() const;
  /** Set the number of vertices of this face.
      \internal */
  void set_n_vertices(unsigned char _n);

  /// Get a halfedge that belongs to this face.
  HalfedgeHandle halfedge_handle()  const;
  /** Set a halfedge that belongs this face.
      \internal */
  void set_halfedge_handle(HalfedgeHandle _eh);
};

};

//=============================================================================
} // namespace Concepts
} // namespace OpenMesh
//=============================================================================

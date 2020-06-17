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

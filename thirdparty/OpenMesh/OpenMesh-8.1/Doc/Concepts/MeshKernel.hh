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
//  Kernel Concept
//
//=============================================================================


#error this file is for documentation purposes only


//== NAMESPACES ===============================================================


namespace OpenMesh {
namespace Concepts {


//== CLASS DEFINITION =========================================================


/** \ingroup mesh_concepts_group
    This class describes the minimum interface a mesh kernel
    has to implement (because the resulting mesh will rely on
    this interface).

    This is the template class the actually holds the mesh kernel
    implementation. All functions marked as internal should only be used
    by the mesh class (that inherits the kernel). The mesh may then
    provide wrapper functions that provide the same functionality.

    \todo Check, if the member list is complete.
*/
template <class FinalMeshItems> class KernelT
{
public:

  /// \name Mesh Items
  //@{

  /// Derive this type from the FinalMeshItems
  typedef typename FinalMeshItems::Vertex             Vertex;
  typedef typename FinalMeshItems::Halfedge           Halfedge;
  typedef typename FinalMeshItems::Edge               Edge;
  typedef typename FinalMeshItems::Face               Face;
  typedef typename FinalMeshItems::Point              Point;
  typedef typename FinalMeshItems::Scalar             Scalar;
  typedef typename FinalMeshItems::Normal             Normal;
  typedef typename FinalMeshItems::Color              Color;
  typedef typename FinalMeshItems::TexCoord           TexCoord;
  typedef typename FinalMeshItems::VertexHandle       VertexHandle;
  typedef typename FinalMeshItems::HalfedgeHandle     HalfedgeHandle;
  typedef typename FinalMeshItems::EdgeHandle         EdgeHandle;
  typedef typename FinalMeshItems::FaceHandle         FaceHandle;

  //@}


  /// \name Kernel Iterators
  //@{
  /// This type depends on the container type in use.
  typedef SomeIterator KernelVertexIter;
  typedef SomeIterator KernelConstVertexIter;
  typedef SomeIterator KernelEdgeIter;
  typedef SomeIterator KernelConstEdgeIter;
  typedef SomeIterator KernelFaceIter;
  typedef SomeIterator KernelConstFaceIter;
  //@}

  /// \name Constructor/Destructor
  //@{
  /// Default constructor
  KernelT() {}

  /// Destructor
  ~KernelT();
  //@}

  /// Assignment operator
  KernelT& operator=(const KernelT& _rhs);


  /** Reserve memory for vertices, edges, faces.
   *
   *  Reserve memory for the mesh items vertices, edges, faces.  Use
   *  this method if you can estimate the memory consumption, for
   *  instance in algorithm expanding the mesh. Depending on the
   *  underlying array type you might be better of using this method,
   *  then letting the array type decide when to increase the
   *  capacity. For instance the STL vector class \c std::vector (used
   *  in the supplied ArrayKernelT) doubles the capacity if it is
   *  exhausted. This might lead to an memory allocation exception,
   *  though an smaller increment would be enough.
   */
  void reserve( size_t _n_vertices,
		size_t _n_edges,
		size_t _n_faces );


  /// \name Handle -> Item.
  //@{
  /// Translate handle to item (see also OpenMesh::PolyMeshT::deref())
  const Vertex& vertex(VertexHandle _h) const { return deref(_h); }
  Vertex& vertex(VertexHandle _h) { return deref(_h); }
  const Halfedge& halfedge(HalfedgeHandle _h) const { return deref(_h); }
  Halfedge& halfedge(HalfedgeHandle _h) { return deref(_h); }
  const Edge& edge(EdgeHandle _h) const { return deref(_h); }
  Edge& edge(EdgeHandle _h) { return deref(_h); }
  const Face& face(FaceHandle _h) const { return deref(_h); }
  Face& face(FaceHandle _h) { return deref(_h); }
  //@}


  /// \name Item -> Handle
  //@{
  /// Translate item to handle
  VertexHandle handle(const Vertex& _v) const;
  HalfedgeHandle handle(const Halfedge& _he) const;
  EdgeHandle handle(const Edge& _e) const;
  FaceHandle handle(const Face& _f) const;
  //@}


  /// \name Get the i'th item
  //@{
  /// Get the i'th item
  VertexHandle   vertex_handle(unsigned int _i) const;
  HalfedgeHandle halfedge_handle(unsigned int _i) const;
  EdgeHandle     edge_handle(unsigned int _i) const;
  FaceHandle     face_handle(unsigned int _i) const;
  //@}


  /// \name Delete items
  //@{
  /**  Delete all items, i.e. clear all item containers.
   *   The properties will also be removed from the mesh
   */
  void clear();

  /**  Delete all items, i.e. clear all item containers.
   *   The properties will be kept
   */
  void clean();

  /** Remove all items that are marked as deleted from the
      corresponding containers.
      \note All handles (and indices) to any entity (face, vertex,
            edge, halfedge) created before garbage collection
            will be out of sync with the mesh, do not use them anymore!
            See also \ref deletedElements.
      \note Needs the Attributes::Status attribute
      \note This function may not be implemented for all kernels.
  */
  void garbage_collection();

  /** Remove the last vertex imidiately, i.e. call pop_back() for the
      VertexContainer.
  */
  void remove_last_vertex() { vertices_.pop_back(); }
  /** Remove the last edge imidiately, i.e. call pop_back() for the
      EdgeContainer. Used e.g. by the add_face() method of PolyMeshT
  */
  void remove_last_edge()   { edges_.pop_back();    }
  /** Remove the last face imidiately, i.e. call pop_back() for the
      FaceContainer. Used e.g. by the add_face() method of PolyMeshT
  */
  void remove_last_face()   { faces_.pop_back();    }

  //@}




  /// \name Number of elements
  //@{
  /// Returns number of vertices
  size_t n_vertices() const;
  /// Returns number of halfedges (should be 2*n_edges())
  size_t n_halfedges() const;
  /// Returns number of edges
  size_t n_edges() const;
  /// Returns number of faces
  size_t n_faces() const;
  /// Is the vertex container empty?
  bool vertices_empty() const;
  /// Is the halfedge container empty (should be the same as edges_empty()).
  bool halfedges_empty() const;
  /// Is the edge container empty?
  bool edges_empty() const;
  /// Is the face container empty?
  bool faces_empty() const;
  //@}



  /// \name Vertex connectivity
  //@{
  /// Get an outgoing halfedge of a given vertex
  HalfedgeHandle halfedge_handle(VertexHandle _vh) const;
  /// Set the outgoing halfedge handle of a given vertex
  void set_halfedge_handle(VertexHandle _vh, HalfedgeHandle _heh);
  /// Get the coordinate of a vertex
  const Point& point(VertexHandle _vh) const;
  /// Get the coordinate of a vertex
  const Point& point(const Vertex& _v) const;
  /// Set the coordinate of a vertex
  void set_point(VertexHandle _vh, const Point& _p);
  /// Set the coordinate of a vertex
  void set_point(Vertex& _v, const Point& _p);
  //@}




  /// \name Halfedge connectivity
  //@{
  /// Get the vertex the halfedge points to
  VertexHandle to_vertex_handle(HalfedgeHandle _heh) const;
  /** Get the vertex the halfedge starts from (implemented as to-handle
      of the opposite halfedge, provided for convenience) */
  VertexHandle from_vertex_handle(HalfedgeHandle _heh) const;
  /// Set the to-vertex-handle of the halfedge
  void set_vertex_handle(HalfedgeHandle _heh, VertexHandle _vh);
  /** Get the face the halfedge belongs to.
      \note The handle is invalid if the halfedge is a boundary halfedge */
  FaceHandle face_handle(HalfedgeHandle _heh) const;
  /// Set the face the halfedge belongs to
  void set_face_handle(HalfedgeHandle _heh, FaceHandle _fh);
  /// Get the next halfedge handle
  HalfedgeHandle next_halfedge_handle(HalfedgeHandle _heh) const;
  /** Set the next halfedge handle.  \note If the previous halfedge is
      also stored (see OpenMesh::Attributes::PrevHalfedge) then this
      method also has to set this link) */
  void set_next_halfedge_handle(HalfedgeHandle _heh, HalfedgeHandle _nheh);
  /** Get the previous halfedge of the given halfedge. The
      implementation should take care of an existing
      OpenMesh::Attributes::PrevHalfedge attribute. */
  HalfedgeHandle prev_halfedge_handle(HalfedgeHandle _heh) const;
  /// Get the opposite halfedge
  HalfedgeHandle opposite_halfedge_handle(HalfedgeHandle _heh) const;
  /// Counter-clockwise rotate the given halfedge around its from vertex
  HalfedgeHandle ccw_rotated_halfedge_handle(HalfedgeHandle _heh) const;
  /// Clockwise rotate the given halfedge around its from vertex
  HalfedgeHandle cw_rotated_halfedge_handle(HalfedgeHandle _heh) const;
  /// Get the edge the current halfedge it contained in
  EdgeHandle edge_handle(HalfedgeHandle _heh) const;
  //@}



  /// \name Edge connectivity
  //@{
  /// Get the first or second halfedge of the given edge
  HalfedgeHandle halfedge_handle(EdgeHandle _eh, unsigned int _i) const;
  //@}



  /// \name Face connectivity
  //@{
  /// Get a halfedge belonging to the face
  HalfedgeHandle halfedge_handle(FaceHandle _fh) const;
  /// Set one halfedge of the face
  void set_halfedge_handle(FaceHandle _fh, HalfedgeHandle _heh);
  //@}


public: // Standard Property Management

  /// \name set/get value of a standard property
  //@{

  // vertex
  const Point& point(VertexHandle _vh) const; ///< Get position
  void set_point(VertexHandle _vh, const Point& _p); ///< Set position
  Point& point(VertexHandle _vh); ///< Convenience function

  const Normal& normal(VertexHandle _vh) const; ///< Get normal
  void set_normal(VertexHandle _vh, const Normal& _n); ///< Set normal

  const Normal& normal(HalfedgeHandle _heh) const; ///< Get normal of the to vertex of the given Halfedge (per face per vertex normals)
  void set_normal(HalfedgeHandle _heh, const Normal& _n); ///< Set normal of the to vertex of the given Halfedge (per face per vertex normals)

  const Color& color(VertexHandle _vh) const; ///< Get color
  void set_color(VertexHandle _vh, const Color& _c); ///< Set color

  const TexCoord1D& texcoord1D(VertexHandle _vh) const; ///< Get texture coordinate.
  void set_texcoord1D(VertexHandle _vh, const TexCoord1D& _t); ///< Set texture coordinate.

  const TexCoord2D& texcoord2D(VertexHandle _vh) const; ///< Get texture coordinate.
  void set_texcoord2D(VertexHandle _vh, const TexCoord2D& _t); ///< Set texture coordinate.

  const TexCoord3D& texcoord3D(VertexHandle _vh) const; ///< Get texture coordinate.
  void set_texcoord3D(VertexHandle _vh, const TexCoord3D& _t); ///< Set texture coordinate.

  const TexCoord1D& texcoord1D(HalfedgeHandle _hh) const; ///< Get texture coordinate of the to vertex for the current face (per face per vertex texcoords)
  void set_texcoord1D(HalfedgeHandle _hh, const TexCoord1D& _t); ///< Set texture coordinate of the to vertex of the given Halfedge (per face per vertex texcoords)

  const TexCoord2D& texcoord2D(HalfedgeHandle _hh) const; ///< Get texture coordinate of the to vertex for the current face (per face per vertex texcoords)
  void set_texcoord2D(HalfedgeHandle _hh, const TexCoord2D& _t); ///< Set texture coordinate of the to vertex of the given Halfedge (per face per vertex texcoords)

  const TexCoord3D& texcoord3D(HalfedgeHandle _hh) const; ///< Get texture coordinate of the to vertex for the current face (per face per vertex texcoords)
  void set_texcoord3D(HalfedgeHandle _hh, const TexCoord3D& _t); ///< Set texture coordinate of the to vertex of the given Halfedge (per face per vertex texcoords)

  const StatusInfo& status(VertexHandle _vh) const; ///< Get status
  StatusInfo& status(VertexHandle _vh); ///< Get status

  // halfedge
  const StatusInfo& status(HalfedgeHandle _vh) const; ///< Get status
  StatusInfo& status(HalfedgeHandle _vh); ///< Get status

  const Color& color(HalfedgeHandle _heh) const; ///< Get color
  void set_color(HalfedgeHandle _heh, const Color& _c); ///< Set color

  // edge
  const Color& color(EdgeHandle _eh) const; ///< Get color
  void set_color(EdgeHandle _eh, const Color& _c); ///< Set color


  const StatusInfo& status(EdgeHandle _vh) const; ///< Get status
  StatusInfo& status(EdgeHandle _vh); ///< Get status

  // face
  const Normal& normal(FaceHandle _fh) const; ///< Get normal
  void set_normal(FaceHandle _fh, const Normal& _n); ///< Set normal

  const Color& color(FaceHandle _fh) const; ///< Get color
  void set_color(FaceHandle _fh, const Color& _c); ///< Set color

  const StatusInfo& status(FaceHandle _vh) const; ///< Get status
  StatusInfo& status(FaceHandle _vh); ///< Get status

  //@}

  /// \name Dynamically add standard properties
  //@{
  /// Request property
  void request_vertex_normals();
  void request_vertex_colors();
  void request_vertex_texcoords1D();
  void request_vertex_texcoords2D();
  void request_vertex_texcoords3D();
  void request_vertex_status();

  void request_halfedge_status();
  void request_halfedge_normals();
  void request_halfedge_colors();
  void request_halfedge_texcoords1D();
  void request_halfedge_texcoords2D();
  void request_halfedge_texcoords3D();

  void request_edge_status();
  void request_edge_colors();

  void request_face_normals();
  void request_face_colors();
  void request_face_status();
  void request_face_texture_index();
  //@}

  /// \name Remove standard properties
  //@{
  /// Remove property
  void release_vertex_normals();
  void release_vertex_colors();
  void release_vertex_texcoords1D();
  void release_vertex_texcoords2D();
  void release_vertex_texcoords3D();
  void release_vertex_status();

  void release_halfedge_status();
  void release_halfedge_normals();
  void release_halfedge_colors();
  void release_halfedge_texcoords1D();
  void release_halfedge_texcoords2D();
  void release_halfedge_texcoords3D();

  void release_edge_status();
  void release_edge_colors();

  void release_face_normals();
  void release_face_colors();
  void release_face_status();
  void release_face_texture_index();
  //@}

  /// \name Check availability of standard properties
  //@{
  /// Is property available?
  bool has_vertex_normals()     const;
  bool has_vertex_colors()      const;
  bool has_vertex_texcoords1D() const;
  bool has_vertex_texcoords2D() const;
  bool has_vertex_texcoords3D() const;
  bool has_vertex_status()      const;

  bool has_halfedge_status()      const;
  bool has_halfedge_normals()     const;
  bool has_halfedge_colors()      const;
  bool has_halfedge_texcoords1D() const;
  bool has_halfedge_texcoords2D() const;
  bool has_halfedge_texcoords3D() const;

  bool has_edge_status() const;
  bool has_edge_colors() const;

  bool has_face_normals() const;
  bool has_face_colors() const;
  bool has_face_status() const;
  bool has_face_texture_index() const;
  //@}

public: // Property Management

  /// \anchor concepts_kernelt_property_management

  /// \name Property management - add property
  //@{
  /// Add property.
  /// @copydoc OpenMesh::BaseKernel::add_property()
  template <typename T> void add_property( [VEHFM]PropHandleT<T>& _ph,
                                           const std::string& _name = "" );
  //@}

  /// \name Property management - remove property
  //@{
  /// Remove property
  template <typename T> void remove_property( [VEHFM]PropHandleT<T>& );
  //@}

  /// \name Property management - get property by name
  //@{
  /// Get property handle by name
  template <typename T>
  bool get_property_handle( [VEHFM]PropHandleT<T>& ph, const std::string& _n ) const;
  //@}

  /// \name Property management - get property
  //@{
  /// Get property
  template <typename T> PropertyT<T>& property( [VEHF]PropHandleT<T> _ph );
  template <typename T> const PropertyT<T>& property( [VEHF]PropHandleT<T> _ph ) const;
  template <typename T> PropertyT<T>& mproperty( MPropHandleT<T> _ph );
  template <typename T> const PropertyT<T>& mproperty( MPropHandleT<T> _ph ) const;
  //@}

  /// \name Property management - get property value for an item
  //@{

  /// Get value for item represented by the handle.
  template <typename T>
  T& property( VPropHandleT<T> _ph, VertexHandle _vh );
  template <typename T>
  const T& property( VPropHandleT<T> _ph, VertexHandle _vh ) const;

  template <typename T>
  T& property( EPropHandleT<T> _ph, EdgeHandle _vh );
  template <typename T>
  const T& property( EPropHandleT<T> _ph, EdgeHandle _vh ) const;

  template <typename T>
  T& property( HPropHandleT<T> _ph, HalfedgeHandle _vh );
  template <typename T>
  const T& property( HPropHandleT<T> _ph, HalfedgeHandle _vh ) const;

  template <typename T>
  T& property( FPropHandleT<T> _ph, FaceHandle _vh );
  template <typename T>
  const T& property( FPropHandleT<T> _ph, FaceHandle _vh ) const;

  template <typename T>
  T& property( MPropHandleT<T> _ph );
  template <typename T>
  const T& property( MPropHandleT<T> _ph ) const;

  //@}

public:


  /// \name Low-level adding new items
  //@{
  /** Add a new (default) vertex.
      \internal */
  VertexHandle new_vertex();
  /** Add a new vertex with a given point coordinate.
      \internal */
  VertexHandle new_vertex(const Point& _p);
  /** Add a new vertex (copied from the given one).
      \internal */
  VertexHandle new_vertex(const Vertex& _v);
  /** Add a new edge from \c _start_vertex_handle to \c _end_vertex_handle.
      This method should add an edge (i.e. two opposite halfedges) and set
      the corresponding vertex handles of these halfedges.
      \internal
   */
  HalfedgeHandle new_edge(VertexHandle _start_vertex_handle,
			  VertexHandle _end_vertex_handle);

  /** Adding a new face
      \internal
  */
  FaceHandle new_face();
  /** Adding a new face (copied from a \c _f).
      \internal */
  FaceHandle new_face(const Face& _f);
  //@}


  // --- iterators ---

  /// \name Kernel item iterators
  //@{
  /** Kernel item iterator
      \internal */
  KernelVertexIter vertices_begin();
  KernelConstVertexIter vertices_begin() const;
  KernelVertexIter vertices_end();
  KernelConstVertexIter vertices_end() const;
  KernelEdgeIter edges_begin();
  KernelConstEdgeIter edges_begin() const;
  KernelEdgeIter edges_end();
  KernelConstEdgeIter edges_end() const;
  KernelFaceIter faces_begin();
  KernelConstFaceIter faces_begin() const;
  KernelFaceIter faces_end();
  KernelConstFaceIter faces_end() const;
  //@}



private:


  // --- private functions ---

  /// copy constructor: not used
  KernelT(const KernelT& _rhs);
};
};


//=============================================================================
} // namespace Concepts
} // namespace OpenMesh
//=============================================================================

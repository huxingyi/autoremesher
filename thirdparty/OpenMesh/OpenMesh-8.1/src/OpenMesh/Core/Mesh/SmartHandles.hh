/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2019, RWTH-Aachen University                 *
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


#pragma once


//== INCLUDES =================================================================

#include "Handles.hh"
#include <OpenMesh/Core/Mesh/PolyConnectivity.hh>
#include <OpenMesh/Core/System/OpenMeshDLLMacros.hh>


//== NAMESPACES ===============================================================

namespace OpenMesh {

//== FORWARD DECLARATION ======================================================

struct SmartVertexHandle;
struct SmartHalfedgeHandle;
struct SmartEdgeHandle;
struct SmartFaceHandle;


//== CLASS DEFINITION =========================================================

/// Base class for all smart handle types
class OPENMESHDLLEXPORT SmartBaseHandle
{
public:
  explicit SmartBaseHandle(const PolyConnectivity* _mesh = nullptr) : mesh_(_mesh) {}

  /// Get the underlying mesh of this handle
  const PolyConnectivity* mesh() const { return mesh_; }

  // TODO: should operators ==, !=, < look at mesh_?

private:
  const PolyConnectivity* mesh_;

};

/// Smart version of VertexHandle contains a pointer to the corresponding mesh and allows easier access to navigation methods
struct OPENMESHDLLEXPORT SmartVertexHandle : public SmartBaseHandle, VertexHandle
{
  explicit SmartVertexHandle(int _idx=-1, const PolyConnectivity* _mesh = nullptr) : SmartBaseHandle(_mesh), VertexHandle(_idx) {}

  /// Returns an outgoing halfedge
  SmartHalfedgeHandle out()      const;
  /// Returns an outgoing halfedge
  SmartHalfedgeHandle halfedge() const; // alias for out
  /// Returns an incoming halfedge
  SmartHalfedgeHandle in()       const;

  /// Returns a range of faces incident to the vertex (PolyConnectivity::vf_range())
  PolyConnectivity::ConstVertexFaceRange      faces()              const;
  /// Returns a range of edges incident to the vertex (PolyConnectivity::ve_range())
  PolyConnectivity::ConstVertexEdgeRange      edges()              const;
  /// Returns a range of vertices adjacent to the vertex (PolyConnectivity::vv_range())
  PolyConnectivity::ConstVertexVertexRange    vertices()           const;
  /// Returns a range of outgoing halfedges incident to the vertex (PolyConnectivity::voh_range())
  PolyConnectivity::ConstVertexIHalfedgeRange incoming_halfedges() const;
  /// Returns a range of incoming halfedges incident to the vertex (PolyConnectivity::vih_range())
  PolyConnectivity::ConstVertexOHalfedgeRange outgoing_halfedges() const;

  /// Returns valence of the vertex
  uint valence()     const;
  /// Returns true iff the vertex is incident to a boundary halfedge
  bool is_boundary() const;
  /// Returns true iff (the mesh at) the vertex is two-manifold ?
  bool is_manifold() const;
};

struct OPENMESHDLLEXPORT SmartHalfedgeHandle : public SmartBaseHandle, HalfedgeHandle
{
  explicit SmartHalfedgeHandle(int _idx=-1, const PolyConnectivity* _mesh = nullptr) : SmartBaseHandle(_mesh), HalfedgeHandle(_idx) {}

  /// Returns next halfedge handle
  SmartHalfedgeHandle next() const;
  /// Returns previous halfedge handle
  SmartHalfedgeHandle prev() const;
  /// Returns opposite halfedge handle
  SmartHalfedgeHandle opp()  const;
  /// Returns vertex pointed to by halfedge
  SmartVertexHandle   to()   const;
  /// Returns vertex at start of halfedge
  SmartVertexHandle   from() const;
  /// Returns incident edge of halfedge
  SmartEdgeHandle     edge() const;
  /// Returns incident face of halfedge
  SmartFaceHandle     face() const;

  /// Returns true iff the halfedge is on the boundary (i.e. it has no corresponding face)
  bool is_boundary() const;
};

struct OPENMESHDLLEXPORT SmartEdgeHandle : public SmartBaseHandle, EdgeHandle
{
  explicit SmartEdgeHandle(int _idx=-1, const PolyConnectivity* _mesh = nullptr) : SmartBaseHandle(_mesh), EdgeHandle(_idx) {}

  /// Returns one of the two halfedges of the edge
  SmartHalfedgeHandle halfedge(unsigned int _i) const;
  /// Shorthand for halfedge()
  SmartHalfedgeHandle h(unsigned int _i)        const;
  /// Shorthand for halfedge(0)
  SmartHalfedgeHandle h0()                      const;
  /// Shorthand for halfedge(1)
  SmartHalfedgeHandle h1()                      const;
  /// Returns one of the two incident vertices of the edge
  SmartVertexHandle   vertex(unsigned int _i)   const;
  /// Shorthand for vertex()
  SmartVertexHandle   v(unsigned int _i)        const;
  /// Shorthand for vertex(0)
  SmartVertexHandle   v0()                      const;
  /// Shorthand for vertex(1)
  SmartVertexHandle   v1()                      const;

  /// Returns true iff the edge lies on the boundary (i.e. one of the halfedges is boundary)
  bool is_boundary() const;
};

struct OPENMESHDLLEXPORT SmartFaceHandle : public SmartBaseHandle, FaceHandle
{
  explicit SmartFaceHandle(int _idx=-1, const PolyConnectivity* _mesh = nullptr) : SmartBaseHandle(_mesh), FaceHandle(_idx) {}

  /// Returns one of the halfedges of the face
  SmartHalfedgeHandle halfedge() const;

  /// Returns a range of vertices incident to the face (PolyConnectivity::fv_range())
  PolyConnectivity::ConstFaceVertexRange   vertices()  const;
  /// Returns a range of halfedges of the face (PolyConnectivity::fh_range())
  PolyConnectivity::ConstFaceHalfedgeRange halfedges() const;
  /// Returns a range of edges of the face (PolyConnectivity::fv_range())
  PolyConnectivity::ConstFaceEdgeRange     edges()     const;
  /// Returns a range adjacent faces of the face (PolyConnectivity::ff_range())
  PolyConnectivity::ConstFaceFaceRange     faces()     const;

  /// Returns the valence of the face
  uint valence()     const;
  /// Returns true iff the face lies at the boundary (i.e. one of the edges is boundary)
  bool is_boundary() const;
};


/// Creats a SmartVertexHandle from a VertexHandle and a Mesh
inline SmartVertexHandle   make_smart(VertexHandle   _vh, const PolyConnectivity* _mesh) { return SmartVertexHandle  (_vh.idx(), _mesh); }
/// Creats a SmartHalfedgeHandle from a HalfedgeHandle and a Mesh
inline SmartHalfedgeHandle make_smart(HalfedgeHandle _hh, const PolyConnectivity* _mesh) { return SmartHalfedgeHandle(_hh.idx(), _mesh); }
/// Creats a SmartEdgeHandle from an EdgeHandle and a Mesh
inline SmartEdgeHandle     make_smart(EdgeHandle     _eh, const PolyConnectivity* _mesh) { return SmartEdgeHandle    (_eh.idx(), _mesh); }
/// Creats a SmartFaceHandle from a FaceHandle and a Mesh
inline SmartFaceHandle     make_smart(FaceHandle     _fh, const PolyConnectivity* _mesh) { return SmartFaceHandle    (_fh.idx(), _mesh); }

/// Creats a SmartVertexHandle from a VertexHandle and a Mesh
inline SmartVertexHandle   make_smart(VertexHandle   _vh, const PolyConnectivity& _mesh) { return SmartVertexHandle  (_vh.idx(), &_mesh); }
/// Creats a SmartHalfedgeHandle from a HalfedgeHandle and a Mesh
inline SmartHalfedgeHandle make_smart(HalfedgeHandle _hh, const PolyConnectivity& _mesh) { return SmartHalfedgeHandle(_hh.idx(), &_mesh); }
/// Creats a SmartEdgeHandle from an EdgeHandle and a Mesh
inline SmartEdgeHandle     make_smart(EdgeHandle     _eh, const PolyConnectivity& _mesh) { return SmartEdgeHandle    (_eh.idx(), &_mesh); }
/// Creats a SmartFaceHandle from a FaceHandle and a Mesh
inline SmartFaceHandle     make_smart(FaceHandle     _fh, const PolyConnectivity& _mesh) { return SmartFaceHandle    (_fh.idx(), &_mesh); }


// helper to convert Handle Types to Smarthandle Types
template <typename HandleT>
struct SmartHandle;

template <> struct SmartHandle<VertexHandle>   { using type = SmartVertexHandle;   };
template <> struct SmartHandle<HalfedgeHandle> { using type = SmartHalfedgeHandle; };
template <> struct SmartHandle<EdgeHandle>     { using type = SmartEdgeHandle;     };
template <> struct SmartHandle<FaceHandle>     { using type = SmartFaceHandle;     };


inline SmartHalfedgeHandle SmartVertexHandle::out() const
{
  assert(mesh() != nullptr);
  return make_smart(mesh()->halfedge_handle(*this), mesh());
}

inline SmartHalfedgeHandle SmartVertexHandle::halfedge() const
{
  return out();
}

inline SmartHalfedgeHandle SmartVertexHandle::in() const
{
  return out().opp();
}

inline uint SmartVertexHandle::valence() const
{
  assert(mesh() != nullptr);
  return mesh()->valence(*this);
}

inline bool SmartVertexHandle::is_boundary() const
{
  assert(mesh() != nullptr);
  return mesh()->is_boundary(*this);
}

inline bool SmartVertexHandle::is_manifold() const
{
  assert(mesh() != nullptr);
  return mesh()->is_manifold(*this);
}

inline SmartHalfedgeHandle SmartHalfedgeHandle::next() const
{
  assert(mesh() != nullptr);
  return make_smart(mesh()->next_halfedge_handle(*this), mesh());
}

inline SmartHalfedgeHandle SmartHalfedgeHandle::prev() const
{
  assert(mesh() != nullptr);
  return make_smart(mesh()->prev_halfedge_handle(*this), mesh());
}

inline SmartHalfedgeHandle SmartHalfedgeHandle::opp() const
{
  assert(mesh() != nullptr);
  return make_smart(mesh()->opposite_halfedge_handle(*this), mesh());
}

inline SmartVertexHandle SmartHalfedgeHandle::to() const
{
  assert(mesh() != nullptr);
  return make_smart(mesh()->to_vertex_handle(*this), mesh());
}

inline SmartVertexHandle SmartHalfedgeHandle::from() const
{
  assert(mesh() != nullptr);
  return make_smart(mesh()->from_vertex_handle(*this), mesh());
}

inline SmartEdgeHandle SmartHalfedgeHandle::edge() const
{
  assert(mesh() != nullptr);
  return make_smart(mesh()->edge_handle(*this), mesh());
}

inline SmartFaceHandle SmartHalfedgeHandle::face() const
{
  assert(mesh() != nullptr);
  return make_smart(mesh()->face_handle(*this), mesh());
}

inline bool SmartHalfedgeHandle::is_boundary() const
{
  assert(mesh() != nullptr);
  return mesh()->is_boundary(*this);
}

inline SmartHalfedgeHandle SmartEdgeHandle::halfedge(unsigned int _i) const
{
  assert(mesh() != nullptr);
  return make_smart(mesh()->halfedge_handle(*this, _i), mesh());
}

inline SmartHalfedgeHandle SmartEdgeHandle::h(unsigned int _i) const
{
  return halfedge(_i);
}

inline SmartHalfedgeHandle SmartEdgeHandle::h0() const
{
  return h(0);
}

inline SmartHalfedgeHandle SmartEdgeHandle::h1() const
{
  return h(1);
}

inline SmartVertexHandle SmartEdgeHandle::vertex(unsigned int _i) const
{
  return halfedge(_i).from();
}

inline SmartVertexHandle SmartEdgeHandle::v(unsigned int _i) const
{
  return vertex(_i);
}

inline SmartVertexHandle SmartEdgeHandle::v0() const
{
  return v(0);
}

inline SmartVertexHandle SmartEdgeHandle::v1() const
{
  return v(1);
}

inline bool SmartEdgeHandle::is_boundary() const
{
  assert(mesh() != nullptr);
  return mesh()->is_boundary(*this);
}

inline SmartHalfedgeHandle SmartFaceHandle::halfedge() const
{
  assert(mesh() != nullptr);
  return make_smart(mesh()->halfedge_handle(*this), mesh());
}

inline uint SmartFaceHandle::valence() const
{
  assert(mesh() != nullptr);
  return mesh()->valence(*this);
}

inline bool SmartFaceHandle::is_boundary() const
{
  assert(mesh() != nullptr);
  return mesh()->is_boundary(*this);
}



//=============================================================================
} // namespace OpenMesh
//=============================================================================

//=============================================================================

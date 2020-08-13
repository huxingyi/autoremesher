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
//  CLASS ArrayKernel
//
//=============================================================================


#ifndef OPENMESH_ARRAY_KERNEL_HH
#define OPENMESH_ARRAY_KERNEL_HH


//== INCLUDES =================================================================
#include <vector>

#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Utils/GenProg.hh>

#include <OpenMesh/Core/Mesh/ArrayItems.hh>
#include <OpenMesh/Core/Mesh/BaseKernel.hh>
#include <OpenMesh/Core/Mesh/Status.hh>

//== NAMESPACES ===============================================================
namespace OpenMesh {


//== CLASS DEFINITION =========================================================
/** \ingroup mesh_kernels_group

    Mesh kernel using arrays for mesh item storage.

    This mesh kernel uses the std::vector as container to store the
    mesh items. Therefore all handle types are internally represented
    by integers. To get the index from a handle use the handle's \c
    idx() method.

    \note For a description of the minimal kernel interface see
    OpenMesh::Mesh::BaseKernel.
    \note You do not have to use this class directly, use the predefined
    mesh-kernel combinations in \ref mesh_types_group.
    \see OpenMesh::Concepts::KernelT, \ref mesh_type
*/

class OPENMESHDLLEXPORT ArrayKernel : public BaseKernel, public ArrayItems
{
public:

  // handles
  typedef OpenMesh::VertexHandle            VertexHandle;
  typedef OpenMesh::HalfedgeHandle          HalfedgeHandle;
  typedef OpenMesh::EdgeHandle              EdgeHandle;
  typedef OpenMesh::FaceHandle              FaceHandle;
  typedef Attributes::StatusInfo            StatusInfo;
  typedef VPropHandleT<StatusInfo>          VertexStatusPropertyHandle;
  typedef HPropHandleT<StatusInfo>          HalfedgeStatusPropertyHandle;
  typedef EPropHandleT<StatusInfo>          EdgeStatusPropertyHandle;
  typedef FPropHandleT<StatusInfo>          FaceStatusPropertyHandle;

public:

  // --- constructor/destructor ---
  ArrayKernel();
  virtual ~ArrayKernel();

  /** ArrayKernel uses the default copy constructor and assignment operator, which means
      that the connectivity and all properties are copied, including reference
      counters, allocated bit status masks, etc.. In contrast assign_connectivity
      copies only the connectivity, i.e. vertices, edges, faces and their status fields.
      NOTE: The geometry (the points property) is NOT copied. Poly/TriConnectivity
      override(and hide) that function to provide connectivity consistence.*/
  void assign_connectivity(const ArrayKernel& _other);

  // --- handle -> item ---
  VertexHandle handle(const Vertex& _v) const;

  HalfedgeHandle handle(const Halfedge& _he) const;

  EdgeHandle handle(const Edge& _e) const;

  FaceHandle handle(const Face& _f) const;


  ///checks handle validity - useful for debugging
  bool is_valid_handle(VertexHandle _vh) const;

  ///checks handle validity - useful for debugging
  bool is_valid_handle(HalfedgeHandle _heh) const;

  ///checks handle validity - useful for debugging
  bool is_valid_handle(EdgeHandle _eh) const;

  ///checks handle validity - useful for debugging
  bool is_valid_handle(FaceHandle _fh) const;


  // --- item -> handle ---
  const Vertex& vertex(VertexHandle _vh) const
  {
    assert(is_valid_handle(_vh));
    return vertices_[_vh.idx()];
  }

  Vertex& vertex(VertexHandle _vh)
  {
    assert(is_valid_handle(_vh));
    return vertices_[_vh.idx()];
  }

  const Halfedge& halfedge(HalfedgeHandle _heh) const
  {
    assert(is_valid_handle(_heh));
    return edges_[_heh.idx() >> 1].halfedges_[_heh.idx() & 1];
  }

  Halfedge& halfedge(HalfedgeHandle _heh)
  {
    assert(is_valid_handle(_heh));
    return edges_[_heh.idx() >> 1].halfedges_[_heh.idx() & 1];
  }

  const Edge& edge(EdgeHandle _eh) const
  {
    assert(is_valid_handle(_eh));
    return edges_[_eh.idx()];
  }

  Edge& edge(EdgeHandle _eh)
  {
    assert(is_valid_handle(_eh));
    return edges_[_eh.idx()];
  }

  const Face& face(FaceHandle _fh) const
  {
    assert(is_valid_handle(_fh));
    return faces_[_fh.idx()];
  }

  Face& face(FaceHandle _fh)
  {
    assert(is_valid_handle(_fh));
    return faces_[_fh.idx()];
  }

  // --- get i'th items ---

  VertexHandle vertex_handle(unsigned int _i) const
  { return (_i < n_vertices()) ? handle( vertices_[_i] ) : VertexHandle(); }

  HalfedgeHandle halfedge_handle(unsigned int _i) const
  {
    return (_i < n_halfedges()) ?
      halfedge_handle(edge_handle(_i/2), _i%2) : HalfedgeHandle();
  }

  EdgeHandle edge_handle(unsigned int _i) const
  { return (_i < n_edges()) ? handle(edges_[_i]) : EdgeHandle(); }

  FaceHandle face_handle(unsigned int _i) const
  { return (_i < n_faces()) ? handle(faces_[_i]) : FaceHandle(); }

public:

  inline VertexHandle new_vertex()
  {
    vertices_.push_back(Vertex());
    vprops_resize(n_vertices());//TODO:should it be push_back()?

    return handle(vertices_.back());
  }

  inline HalfedgeHandle new_edge(VertexHandle _start_vh, VertexHandle _end_vh)
  {
//     assert(_start_vh != _end_vh);
    edges_.push_back(Edge());
    eprops_resize(n_edges());//TODO:should it be push_back()?
    hprops_resize(n_halfedges());//TODO:should it be push_back()?

    EdgeHandle eh(handle(edges_.back()));
    HalfedgeHandle heh0(halfedge_handle(eh, 0));
    HalfedgeHandle heh1(halfedge_handle(eh, 1));
    set_vertex_handle(heh0, _end_vh);
    set_vertex_handle(heh1, _start_vh);
    return heh0;
  }

  inline FaceHandle new_face()
  {
    faces_.push_back(Face());
    fprops_resize(n_faces());
    return handle(faces_.back());
  }

  inline FaceHandle new_face(const Face& _f)
  {
    faces_.push_back(_f);
    fprops_resize(n_faces());
    return handle(faces_.back());
  }

public:
  // --- resize/reserve ---
  void resize( size_t _n_vertices, size_t _n_edges, size_t _n_faces );
  void reserve(size_t _n_vertices, size_t _n_edges, size_t _n_faces );

  // --- deletion ---
  /** \brief garbage collection
   *
   * Usually if you delete primitives in OpenMesh, they are only flagged as deleted.
   * Only when you call garbage collection, they will be actually removed.
   *
   * \note Garbage collection invalidates all handles. If you need to keep track of
   *       a set of handles, you can pass them to the second garbage collection
   *       function, which will update a vector of handles.
   *       See also \ref deletedElements.
   *
   * @param _v Remove deleted vertices?
   * @param _e Remove deleted edges?
   * @param _f Remove deleted faces?
   *
   */
  void garbage_collection(bool _v=true, bool _e=true, bool _f=true);

  /** \brief garbage collection with handle tracking
   *
   * Usually if you delete primitives in OpenMesh, they are only flagged as deleted.
   * Only when you call garbage collection, they will be actually removed.
   *
   * \note Garbage collection invalidates all handles. If you need to keep track of
   *       a set of handles, you can pass them to this function. The handles that the
   *       given pointers point to are updated in place.
   *       See also \ref deletedElements.
   *
   * @param vh_to_update Pointers to vertex handles that should get updated
   * @param hh_to_update Pointers to halfedge handles that should get updated
   * @param fh_to_update Pointers to face handles that should get updated
   * @param _v Remove deleted vertices?
   * @param _e Remove deleted edges?
   * @param _f Remove deleted faces?
   */
  template<typename std_API_Container_VHandlePointer,
           typename std_API_Container_HHandlePointer,
           typename std_API_Container_FHandlePointer>
  void garbage_collection(std_API_Container_VHandlePointer& vh_to_update,
                          std_API_Container_HHandlePointer& hh_to_update,
                          std_API_Container_FHandlePointer& fh_to_update,
                          bool _v=true, bool _e=true, bool _f=true);

  /** \brief Clear the whole mesh
   *
   *  This will remove all properties and elements from the mesh
   */
  void clear();

  /** \brief Reset the whole mesh
   *
   *  This will remove all elements from the mesh but keeps the properties
   */
  void clean();

  // --- number of items ---
  size_t n_vertices()  const { return vertices_.size(); }
  size_t n_halfedges() const { return 2*edges_.size(); }
  size_t n_edges()     const { return edges_.size(); }
  size_t n_faces()     const { return faces_.size(); }

  bool vertices_empty()  const { return vertices_.empty(); }
  bool halfedges_empty() const { return edges_.empty(); }
  bool edges_empty()     const { return edges_.empty(); }
  bool faces_empty()     const { return faces_.empty(); }

  // --- vertex connectivity ---

  HalfedgeHandle halfedge_handle(VertexHandle _vh) const
  { return vertex(_vh).halfedge_handle_; }

  void set_halfedge_handle(VertexHandle _vh, HalfedgeHandle _heh)
  {
//     assert(is_valid_handle(_heh));
    vertex(_vh).halfedge_handle_ = _heh;
  }

  bool is_isolated(VertexHandle _vh) const
  { return !halfedge_handle(_vh).is_valid(); }

  void set_isolated(VertexHandle _vh)
  { vertex(_vh).halfedge_handle_.invalidate(); }

  unsigned int delete_isolated_vertices();

  // --- halfedge connectivity ---
  VertexHandle to_vertex_handle(HalfedgeHandle _heh) const
  { return halfedge(_heh).vertex_handle_; }

  VertexHandle from_vertex_handle(HalfedgeHandle _heh) const
  { return to_vertex_handle(opposite_halfedge_handle(_heh)); }

  void set_vertex_handle(HalfedgeHandle _heh, VertexHandle _vh)
  {
//     assert(is_valid_handle(_vh));
    halfedge(_heh).vertex_handle_ = _vh;
  }

  FaceHandle face_handle(HalfedgeHandle _heh) const
  { return halfedge(_heh).face_handle_; }

  void set_face_handle(HalfedgeHandle _heh, FaceHandle _fh)
  {
//     assert(is_valid_handle(_fh));
    halfedge(_heh).face_handle_ = _fh;
  }

  void set_boundary(HalfedgeHandle _heh)
  { halfedge(_heh).face_handle_.invalidate(); }

  /// Is halfedge _heh a boundary halfedge (is its face handle invalid) ?
  bool is_boundary(HalfedgeHandle _heh) const
  { return !face_handle(_heh).is_valid(); }

  HalfedgeHandle next_halfedge_handle(HalfedgeHandle _heh) const
  { return halfedge(_heh).next_halfedge_handle_; }

  void set_next_halfedge_handle(HalfedgeHandle _heh, HalfedgeHandle _nheh)
  {
    //assert(is_valid_handle(_nheh));
//     assert(to_vertex_handle(_heh) == from_vertex_handle(_nheh));
    halfedge(_heh).next_halfedge_handle_ = _nheh;
    if (is_valid_handle(_nheh)) // fixed by AutoRemesher
      set_prev_halfedge_handle(_nheh, _heh);
  }


  void set_prev_halfedge_handle(HalfedgeHandle _heh, HalfedgeHandle _pheh)
  {
    assert(is_valid_handle(_pheh));
    set_prev_halfedge_handle(_heh, _pheh, HasPrevHalfedge());
  }

  void set_prev_halfedge_handle(HalfedgeHandle _heh, HalfedgeHandle _pheh,
                                GenProg::True)
  { halfedge(_heh).prev_halfedge_handle_ = _pheh; }

  void set_prev_halfedge_handle(HalfedgeHandle /* _heh */, HalfedgeHandle /* _pheh */,
                                GenProg::False)
  {}

  HalfedgeHandle prev_halfedge_handle(HalfedgeHandle _heh) const
  { return prev_halfedge_handle(_heh, HasPrevHalfedge() ); }

  HalfedgeHandle prev_halfedge_handle(HalfedgeHandle _heh, GenProg::True) const
  { return halfedge(_heh).prev_halfedge_handle_; }

  HalfedgeHandle prev_halfedge_handle(HalfedgeHandle _heh, GenProg::False) const
  {
    if (is_boundary(_heh))
    {//iterating around the vertex should be faster than iterating the boundary
      HalfedgeHandle curr_heh(opposite_halfedge_handle(_heh));
      HalfedgeHandle next_heh(next_halfedge_handle(curr_heh));
      do
      {
        curr_heh = opposite_halfedge_handle(next_heh);
        next_heh = next_halfedge_handle(curr_heh);
      }
      while (next_heh != _heh);
      return curr_heh;
    }
    else
    {
      HalfedgeHandle  heh(_heh);
      HalfedgeHandle  next_heh(next_halfedge_handle(heh));
      while (next_heh != _heh) {
        heh = next_heh;
        next_heh = next_halfedge_handle(next_heh);
      }
      return heh;
    }
  }


  HalfedgeHandle opposite_halfedge_handle(HalfedgeHandle _heh) const
  { return HalfedgeHandle((_heh.idx() & 1) ? _heh.idx()-1 : _heh.idx()+1); }


  HalfedgeHandle ccw_rotated_halfedge_handle(HalfedgeHandle _heh) const
  { return opposite_halfedge_handle(prev_halfedge_handle(_heh)); }


  HalfedgeHandle cw_rotated_halfedge_handle(HalfedgeHandle _heh) const
  { return next_halfedge_handle(opposite_halfedge_handle(_heh)); }

  // --- edge connectivity ---
  HalfedgeHandle halfedge_handle(EdgeHandle _eh, unsigned int _i) const
  {
    assert(_i<=1);
    return HalfedgeHandle((_eh.idx() << 1) + _i);
  }

  EdgeHandle edge_handle(HalfedgeHandle _heh) const
  { return EdgeHandle(_heh.idx() >> 1); }

  // --- face connectivity ---
  HalfedgeHandle halfedge_handle(FaceHandle _fh) const
  { return face(_fh).halfedge_handle_; }

  void set_halfedge_handle(FaceHandle _fh, HalfedgeHandle _heh)
  {
//     assert(is_valid_handle(_heh));
    face(_fh).halfedge_handle_ = _heh;
  }

  /// Status Query API
  //------------------------------------------------------------ vertex status
  const StatusInfo&                         status(VertexHandle _vh) const
  { return property(vertex_status_, _vh); }

  StatusInfo&                               status(VertexHandle _vh)
  { return property(vertex_status_, _vh); }

  //----------------------------------------------------------- halfedge status
  const StatusInfo&                         status(HalfedgeHandle _hh) const
  { return property(halfedge_status_, _hh);  }

  StatusInfo&                               status(HalfedgeHandle _hh)
  { return property(halfedge_status_, _hh); }

  //--------------------------------------------------------------- edge status
  const StatusInfo&                         status(EdgeHandle _eh) const
  { return property(edge_status_, _eh); }

  StatusInfo&                               status(EdgeHandle _eh)
  { return property(edge_status_, _eh); }

  //--------------------------------------------------------------- face status
  const StatusInfo&                         status(FaceHandle _fh) const
  { return property(face_status_, _fh); }

  StatusInfo&                               status(FaceHandle _fh)
  { return property(face_status_, _fh); }

  inline bool                               has_vertex_status() const
  { return vertex_status_.is_valid();    }

  inline bool                               has_halfedge_status() const
  { return halfedge_status_.is_valid();  }

  inline bool                               has_edge_status() const
  { return edge_status_.is_valid(); }

  inline bool                               has_face_status() const
  { return face_status_.is_valid(); }

  inline VertexStatusPropertyHandle         vertex_status_pph() const
  { return vertex_status_;  }

  inline HalfedgeStatusPropertyHandle       halfedge_status_pph() const
  { return halfedge_status_; }

  inline EdgeStatusPropertyHandle           edge_status_pph() const
  { return edge_status_;  }

  inline FaceStatusPropertyHandle           face_status_pph() const
  { return face_status_; }

  /// status property by handle
  inline VertexStatusPropertyHandle         status_pph(VertexHandle /*_hnd*/) const
  { return vertex_status_pph(); }

  inline HalfedgeStatusPropertyHandle       status_pph(HalfedgeHandle /*_hnd*/) const
  { return halfedge_status_pph(); }

  inline EdgeStatusPropertyHandle           status_pph(EdgeHandle /*_hnd*/) const
  { return edge_status_pph();  }

  inline FaceStatusPropertyHandle           status_pph(FaceHandle /*_hnd*/) const
  { return face_status_pph();  }

  /// Status Request API
  void request_vertex_status()
  {
    if (!refcount_vstatus_++)
      add_property( vertex_status_, "v:status" );
  }

  void request_halfedge_status()
  {
    if (!refcount_hstatus_++)
      add_property( halfedge_status_, "h:status" );
  }

  void request_edge_status()
  {
    if (!refcount_estatus_++)
      add_property( edge_status_, "e:status" );
  }

  void request_face_status()
  {
    if (!refcount_fstatus_++)
      add_property( face_status_, "f:status" );
  }

  /// Status Release API
  void release_vertex_status()
  {
    if ((refcount_vstatus_ > 0) && (! --refcount_vstatus_))
      remove_property(vertex_status_);
  }

  void release_halfedge_status()
  {
    if ((refcount_hstatus_ > 0) && (! --refcount_hstatus_))
      remove_property(halfedge_status_);
  }

  void release_edge_status()
  {
    if ((refcount_estatus_ > 0) && (! --refcount_estatus_))
      remove_property(edge_status_);
  }

  void release_face_status()
  {
    if ((refcount_fstatus_ > 0) && (! --refcount_fstatus_))
      remove_property(face_status_);
  }

  /// --- StatusSet API ---

  template <class Handle>
  class StatusSetT
  {
  protected:
    ArrayKernel&                            kernel_;

  public:
    const unsigned int                      bit_mask_;

  public:
    StatusSetT(ArrayKernel& _kernel, unsigned int _bit_mask)
    : kernel_(_kernel), bit_mask_(_bit_mask)
    {}

    ~StatusSetT()
    {}

    inline bool                             is_in(Handle _hnd) const
    { return kernel_.status(_hnd).is_bit_set(bit_mask_); }

    inline void                             insert(Handle _hnd)
    { kernel_.status(_hnd).set_bit(bit_mask_); }

    inline void                             erase(Handle _hnd)
    { kernel_.status(_hnd).unset_bit(bit_mask_); }

    /// Note: 0(n) complexity
    unsigned int                            size() const
    {
      unsigned int n_elements = kernel_.status_pph(Handle()).is_valid() ?
                                kernel_.property(kernel_.status_pph(Handle())).n_elements() : 0;
      unsigned int sz = 0;
      for (unsigned int i = 0; i < n_elements; ++i)
      {
        sz += (unsigned int)is_in(Handle(i));
      }
      return sz;
    }

    /// Note: O(n) complexity
    void                                    clear()
    {
      unsigned int n_elements = kernel_.status_pph(Handle()).is_valid() ?
                                kernel_.property(kernel_.status_pph(Handle())).n_elements() : 0;
      for (unsigned int i = 0; i < n_elements; ++i)
      {
        erase(Handle(i));
      }
    }
  };

  friend class StatusSetT<VertexHandle>;
  friend class StatusSetT<EdgeHandle>;
  friend class StatusSetT<FaceHandle>;
  friend class StatusSetT<HalfedgeHandle>;

  /// --- AutoStatusSet API ---

  template <class Handle>
  class AutoStatusSetT : public StatusSetT<Handle>
  {
  private:
    typedef StatusSetT<Handle>              Base;
  public:
    AutoStatusSetT(ArrayKernel& _kernel)
    : StatusSetT<Handle>(_kernel, _kernel.pop_bit_mask(Handle()))
    { /*assert(size() == 0);*/ } //the set should be empty on creation

    ~AutoStatusSetT()
    {
      //assert(size() == 0);//the set should be empty on leave?
      Base::kernel_.push_bit_mask(Handle(), Base::bit_mask_);
    }
  };

  friend class AutoStatusSetT<VertexHandle>;
  friend class AutoStatusSetT<EdgeHandle>;
  friend class AutoStatusSetT<FaceHandle>;
  friend class AutoStatusSetT<HalfedgeHandle>;

  typedef AutoStatusSetT<VertexHandle>      VertexStatusSet;
  typedef AutoStatusSetT<EdgeHandle>        EdgeStatusSet;
  typedef AutoStatusSetT<FaceHandle>        FaceStatusSet;
  typedef AutoStatusSetT<HalfedgeHandle>    HalfedgeStatusSet;

  /// --- ExtStatusSet API --- (hybrid between a set and an array)

  template <class Handle>
  class ExtStatusSetT : public AutoStatusSetT<Handle>
  {
  public:
    typedef AutoStatusSetT<Handle>          Base;

  protected:
    typedef std::vector<Handle>             HandleContainer;
    HandleContainer                         handles_;

  public:
    typedef typename HandleContainer::iterator
                                            iterator;
    typedef typename HandleContainer::const_iterator
                                            const_iterator;
  public:
    ExtStatusSetT(ArrayKernel& _kernel, size_t _capacity_hint = 0)
    : Base(_kernel)
    { handles_.reserve(_capacity_hint); }

    ~ExtStatusSetT()
    { clear(); }

    //set API
    // Complexity: O(1)
    inline void                             insert(Handle _hnd)
    {
      if (!is_in(_hnd))
      {
        Base::insert(_hnd);
        handles_.push_back(_hnd);
      }
    }

    // Complexity: O(k), (k - number of the elements in the set)
    inline void                             erase(Handle _hnd)
    {
      if (is_in(_hnd))
      {
        iterator it = std::find(begin(), end(), _hnd);
        erase(it);
      }
    }

    // Complexity: O(1)
    inline void                             erase(iterator _it)
    {
      assert(_it != end() && is_in(*_it));
      clear(*_it);
      *_it = handles_.back();
      _it.pop_back();
    }

    inline void                             clear()
    {
      for (iterator it = begin(); it != end(); ++it)
      {
        assert(is_in(*it));
        Base::erase(*it);
      }
      handles_.clear();
    }

    /// Complexity: 0(1)
    inline unsigned int                     size() const
    { return handles_.size(); }
    inline bool                             empty() const
    { return handles_.empty(); }

    //Vector API
    inline iterator                         begin()
    { return handles_.begin(); }
    inline const_iterator                   begin() const
    { return handles_.begin(); }

    inline iterator                         end()
    { return handles_.end(); }
    inline const_iterator                   end() const
    { return handles_.end(); }

    inline Handle&                          front()
    { return handles_.front(); }
    inline const Handle&                    front() const
    { return handles_.front(); }

    inline Handle&                          back()
    { return handles_.back(); }
    inline const Handle&                    back() const
    { return handles_.back(); }
  };

  typedef ExtStatusSetT<FaceHandle>         ExtFaceStatusSet;
  typedef ExtStatusSetT<VertexHandle>       ExtVertexStatusSet;
  typedef ExtStatusSetT<EdgeHandle>         ExtEdgeStatusSet;
  typedef ExtStatusSetT<HalfedgeHandle>     ExtHalfedgeStatusSet;

private:
  // iterators
  typedef std::vector<Vertex>                VertexContainer;
  typedef std::vector<Edge>                  EdgeContainer;
  typedef std::vector<Face>                  FaceContainer;
  typedef VertexContainer::iterator          KernelVertexIter;
  typedef VertexContainer::const_iterator    KernelConstVertexIter;
  typedef EdgeContainer::iterator            KernelEdgeIter;
  typedef EdgeContainer::const_iterator      KernelConstEdgeIter;
  typedef FaceContainer::iterator            KernelFaceIter;
  typedef FaceContainer::const_iterator      KernelConstFaceIter;
  typedef std::vector<unsigned int>          BitMaskContainer;


  KernelVertexIter      vertices_begin()        { return vertices_.begin(); }
  KernelConstVertexIter vertices_begin() const  { return vertices_.begin(); }
  KernelVertexIter      vertices_end()          { return vertices_.end(); }
  KernelConstVertexIter vertices_end() const    { return vertices_.end(); }

  KernelEdgeIter        edges_begin()           { return edges_.begin(); }
  KernelConstEdgeIter   edges_begin() const     { return edges_.begin(); }
  KernelEdgeIter        edges_end()             { return edges_.end(); }
  KernelConstEdgeIter   edges_end() const       { return edges_.end(); }

  KernelFaceIter        faces_begin()           { return faces_.begin(); }
  KernelConstFaceIter   faces_begin() const     { return faces_.begin(); }
  KernelFaceIter        faces_end()             { return faces_.end(); }
  KernelConstFaceIter   faces_end() const       { return faces_.end(); }

  /// bit mask container by handle
  inline BitMaskContainer&                  bit_masks(VertexHandle /*_dummy_hnd*/)
  { return vertex_bit_masks_; }
  inline BitMaskContainer&                  bit_masks(EdgeHandle /*_dummy_hnd*/)
  { return edge_bit_masks_; }
  inline BitMaskContainer&                  bit_masks(FaceHandle /*_dummy_hnd*/)
  { return face_bit_masks_; }
  inline BitMaskContainer&                  bit_masks(HalfedgeHandle /*_dummy_hnd*/)
  { return halfedge_bit_masks_; }

  template <class Handle>
  unsigned int                              pop_bit_mask(Handle _hnd)
  {
    assert(!bit_masks(_hnd).empty());//check if the client request too many status sets
    unsigned int bit_mask = bit_masks(_hnd).back();
    bit_masks(_hnd).pop_back();
    return bit_mask;
  }

  template <class Handle>
  void                                      push_bit_mask(Handle _hnd, unsigned int _bit_mask)
  {
    assert(std::find(bit_masks(_hnd).begin(), bit_masks(_hnd).end(), _bit_mask) ==
           bit_masks(_hnd).end());//this mask should be not already used
    bit_masks(_hnd).push_back(_bit_mask);
  }

  void                                      init_bit_masks(BitMaskContainer& _bmc);
  void                                      init_bit_masks();

private:
  VertexContainer                           vertices_;
  EdgeContainer                             edges_;
  FaceContainer                             faces_;

  VertexStatusPropertyHandle                vertex_status_;
  HalfedgeStatusPropertyHandle              halfedge_status_;
  EdgeStatusPropertyHandle                  edge_status_;
  FaceStatusPropertyHandle                  face_status_;

  unsigned int                              refcount_vstatus_;
  unsigned int                              refcount_hstatus_;
  unsigned int                              refcount_estatus_;
  unsigned int                              refcount_fstatus_;

  BitMaskContainer                          halfedge_bit_masks_;
  BitMaskContainer                          edge_bit_masks_;
  BitMaskContainer                          vertex_bit_masks_;
  BitMaskContainer                          face_bit_masks_;
};


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_ARRAY_KERNEL_C)
#  define OPENMESH_ARRAY_KERNEL_TEMPLATES
#  include "ArrayKernelT.cc"
#endif
//=============================================================================
#endif // OPENMESH_ARRAY_KERNEL_HH defined
//=============================================================================

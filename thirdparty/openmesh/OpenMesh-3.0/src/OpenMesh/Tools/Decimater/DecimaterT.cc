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

/** \file DecimaterT.cc
 */

//=============================================================================
//
//  CLASS DecimaterT - IMPLEMENTATION
//
//=============================================================================
#define OPENMESH_DECIMATER_DECIMATERT_CC

//== INCLUDES =================================================================

#include <OpenMesh/Tools/Decimater/DecimaterT.hh>

#include <vector>
#if defined(OM_CC_MIPS)
#  include <float.h>
#else
#  include <cfloat>
#endif

//== NAMESPACE ===============================================================

namespace OpenMesh {
namespace Decimater {

//== IMPLEMENTATION ==========================================================

template<class Mesh>
DecimaterT<Mesh>::DecimaterT(Mesh& _mesh) :
  BaseDecimaterT<Mesh>(_mesh),
    mesh_(_mesh), heap_(NULL) {

  // private vertex properties
  mesh_.add_property(collapse_target_);
  mesh_.add_property(priority_);
  mesh_.add_property(heap_position_);
}

//-----------------------------------------------------------------------------

template<class Mesh>
DecimaterT<Mesh>::~DecimaterT() {

  // private vertex properties
  mesh_.remove_property(collapse_target_);
  mesh_.remove_property(priority_);
  mesh_.remove_property(heap_position_);

}

//-----------------------------------------------------------------------------

template<class Mesh>
void DecimaterT<Mesh>::heap_vertex(VertexHandle _vh) {
  //   std::clog << "heap_vertex: " << _vh << std::endl;

  float prio, best_prio(FLT_MAX);
  typename Mesh::HalfedgeHandle heh, collapse_target;

  // find best target in one ring
  typename Mesh::VertexOHalfedgeIter voh_it(mesh_, _vh);
  for (; voh_it.is_valid(); ++voh_it) {
    heh = *voh_it;
    CollapseInfo ci(mesh_, heh);

    if (this->is_collapse_legal(ci)) {
      prio = this->collapse_priority(ci);
      if (prio >= 0.0 && prio < best_prio) {
        best_prio = prio;
        collapse_target = heh;
      }
    }
  }

  // target found -> put vertex on heap
  if (collapse_target.is_valid()) {
    //     std::clog << "  added|updated" << std::endl;
    mesh_.property(collapse_target_, _vh) = collapse_target;
    mesh_.property(priority_, _vh)        = best_prio;

    if (heap_->is_stored(_vh))
      heap_->update(_vh);
    else
      heap_->insert(_vh);
  }

  // not valid -> remove from heap
  else {
    //     std::clog << "  n/a|removed" << std::endl;
    if (heap_->is_stored(_vh))
      heap_->remove(_vh);

    mesh_.property(collapse_target_, _vh) = collapse_target;
    mesh_.property(priority_, _vh) = -1;
  }
}

//-----------------------------------------------------------------------------
template<class Mesh>
size_t DecimaterT<Mesh>::decimate(size_t _n_collapses) {

  if (!this->is_initialized())
    return 0;

  typename Mesh::VertexIter v_it, v_end(mesh_.vertices_end());
  typename Mesh::VertexHandle vp;
  typename Mesh::HalfedgeHandle v0v1;
  typename Mesh::VertexVertexIter vv_it;
  typename Mesh::VertexFaceIter vf_it;
  unsigned int n_collapses(0);

  typedef std::vector<typename Mesh::VertexHandle> Support;
  typedef typename Support::iterator SupportIterator;

  Support support(15);
  SupportIterator s_it, s_end;

  // check _n_collapses
  if (!_n_collapses)
    _n_collapses = mesh_.n_vertices();

  // initialize heap
  HeapInterface HI(mesh_, priority_, heap_position_);
  heap_ = std::auto_ptr<DeciHeap>(new DeciHeap(HI));
  heap_->reserve(mesh_.n_vertices());

  for (v_it = mesh_.vertices_begin(); v_it != v_end; ++v_it) {
    heap_->reset_heap_position(*v_it);
    if (!mesh_.status(*v_it).deleted())
      heap_vertex(*v_it);
  }

  // process heap
  while ((!heap_->empty()) && (n_collapses < _n_collapses)) {
    // get 1st heap entry
    vp = heap_->front();
    v0v1 = mesh_.property(collapse_target_, vp);
    heap_->pop_front();

    // setup collapse info
    CollapseInfo ci(mesh_, v0v1);

    // check topological correctness AGAIN !
    if (!this->is_collapse_legal(ci))
      continue;

    // store support (= one ring of *vp)
    vv_it = mesh_.vv_iter(ci.v0);
    support.clear();
    for (; vv_it.is_valid(); ++vv_it)
      support.push_back(*vv_it);

    // perform collapse
    mesh_.collapse(v0v1);
    ++n_collapses;

    // update triangle normals
    vf_it = mesh_.vf_iter(ci.v1);
    for (; vf_it.is_valid(); ++vf_it)
      if (!mesh_.status(*vf_it).deleted())
        mesh_.set_normal(*vf_it, mesh_.calc_face_normal(*vf_it));

    // post-process collapse
    this->postprocess_collapse(ci);

    // update heap (former one ring of decimated vertex)
    for (s_it = support.begin(), s_end = support.end(); s_it != s_end; ++s_it) {
      assert(!mesh_.status(*s_it).deleted());
      heap_vertex(*s_it);
    }
  }

  // delete heap
  heap_.reset();



  // DON'T do garbage collection here! It's up to the application.
  return n_collapses;
}

//-----------------------------------------------------------------------------

template<class Mesh>
size_t DecimaterT<Mesh>::decimate_to_faces(size_t _nv, size_t _nf) {

  if (!this->is_initialized())
    return 0;

  if (_nv >= mesh_.n_vertices() || _nf >= mesh_.n_faces())
    return 0;

  typename Mesh::VertexIter v_it, v_end(mesh_.vertices_end());
  typename Mesh::VertexHandle vp;
  typename Mesh::HalfedgeHandle v0v1;
  typename Mesh::VertexVertexIter vv_it;
  typename Mesh::VertexFaceIter vf_it;
  size_t nv = mesh_.n_vertices();
  size_t nf = mesh_.n_faces();
  unsigned int n_collapses = 0;

  typedef std::vector<typename Mesh::VertexHandle> Support;
  typedef typename Support::iterator SupportIterator;

  Support support(15);
  SupportIterator s_it, s_end;

  // initialize heap
  HeapInterface HI(mesh_, priority_, heap_position_);
  heap_ = std::auto_ptr<DeciHeap>(new DeciHeap(HI));
  heap_->reserve(mesh_.n_vertices());

  for (v_it = mesh_.vertices_begin(); v_it != v_end; ++v_it) {
    heap_->reset_heap_position(*v_it);
    if (!mesh_.status(*v_it).deleted())
      heap_vertex(*v_it);
  }

  // process heap
  while ((!heap_->empty()) && (_nv < nv) && (_nf < nf)) {
    // get 1st heap entry
    vp = heap_->front();
    v0v1 = mesh_.property(collapse_target_, vp);
    heap_->pop_front();

    // setup collapse info
    CollapseInfo ci(mesh_, v0v1);

    // check topological correctness AGAIN !
    if (!this->is_collapse_legal(ci))
      continue;

    // store support (= one ring of *vp)
    vv_it = mesh_.vv_iter(ci.v0);
    support.clear();
    for (; vv_it.is_valid(); ++vv_it)
      support.push_back(*vv_it);

    // adjust complexity in advance (need boundary status)
    ++n_collapses;
    --nv;
    if (mesh_.is_boundary(ci.v0v1) || mesh_.is_boundary(ci.v1v0))
      --nf;
    else
      nf -= 2;

    // pre-processing
    this->preprocess_collapse(ci);

    // perform collapse
    mesh_.collapse(v0v1);

    // update triangle normals
    vf_it = mesh_.vf_iter(ci.v1);
    for (; vf_it.is_valid(); ++vf_it)
      if (!mesh_.status(*vf_it).deleted())
        mesh_.set_normal(*vf_it, mesh_.calc_face_normal(*vf_it));

    // post-process collapse
    this->postprocess_collapse(ci);

    // update heap (former one ring of decimated vertex)
    for (s_it = support.begin(), s_end = support.end(); s_it != s_end; ++s_it) {
      assert(!mesh_.status(*s_it).deleted());
      heap_vertex(*s_it);
    }
  }

  // delete heap
  heap_.reset();


  // DON'T do garbage collection here! It's up to the application.
  return n_collapses;
}

//=============================================================================
}// END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================


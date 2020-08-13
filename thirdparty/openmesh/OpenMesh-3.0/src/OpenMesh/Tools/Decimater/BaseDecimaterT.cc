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
#define OPENMESH_BASE_DECIMATER_DECIMATERT_CC

//== INCLUDES =================================================================

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
BaseDecimaterT<Mesh>::BaseDecimaterT(Mesh& _mesh) :
    mesh_(_mesh), cmodule_(NULL), initialized_(false) {
  // default properties
  mesh_.request_vertex_status();
  mesh_.request_edge_status();
  mesh_.request_face_status();
  mesh_.request_face_normals();
}

//-----------------------------------------------------------------------------

template<class Mesh>
BaseDecimaterT<Mesh>::~BaseDecimaterT() {
  // default properties
  mesh_.release_vertex_status();
  mesh_.release_edge_status();
  mesh_.release_face_status();
  mesh_.release_face_normals();

  // dispose of modules
  {
    set_uninitialized();
    typename ModuleList::iterator m_it, m_end = all_modules_.end();
    for (m_it = all_modules_.begin(); m_it != m_end; ++m_it)
      delete *m_it;
    all_modules_.clear();
  }
}

//-----------------------------------------------------------------------------

template<class Mesh>
bool BaseDecimaterT<Mesh>::is_collapse_legal(const CollapseInfo& _ci) {
  //   std::clog << "McDecimaterT<>::is_collapse_legal()\n";

  // locked ?
  if (mesh_.status(_ci.v0).locked())
    return false;

  // this test checks:
  // is v0v1 deleted?
  // is v0 deleted?
  // is v1 deleted?
  // are both vlv0 and v1vl boundary edges?
  // are both v0vr and vrv1 boundary edges?
  // are vl and vr equal or both invalid?
  // one ring intersection test
  // edge between two boundary vertices should be a boundary edge
  if (!mesh_.is_collapse_ok(_ci.v0v1))
    return false;

  if (_ci.vl.is_valid() && _ci.vr.is_valid()
      && mesh_.find_halfedge(_ci.vl, _ci.vr).is_valid()
      && mesh_.valence(_ci.vl) == 3 && mesh_.valence(_ci.vr) == 3) {
    return false;
  }
  //--- feature test ---

  if (mesh_.status(_ci.v0).feature()
      && !mesh_.status(mesh_.edge_handle(_ci.v0v1)).feature())
    return false;

  //--- test boundary cases ---
  if (mesh_.is_boundary(_ci.v0)) {

    // don't collapse a boundary vertex to an inner one
    if (!mesh_.is_boundary(_ci.v1))
      return false;

    // only one one ring intersection
    if (_ci.vl.is_valid() && _ci.vr.is_valid())
      return false;
  }

  // there have to be at least 2 incident faces at v0
  if (mesh_.cw_rotated_halfedge_handle(
      mesh_.cw_rotated_halfedge_handle(_ci.v0v1)) == _ci.v0v1)
    return false;

  // collapse passed all tests -> ok
  return true;
}

//-----------------------------------------------------------------------------

template<class Mesh>
float BaseDecimaterT<Mesh>::collapse_priority(const CollapseInfo& _ci) {
  typename ModuleList::iterator m_it, m_end = bmodules_.end();

  for (m_it = bmodules_.begin(); m_it != m_end; ++m_it) {
    if ((*m_it)->collapse_priority(_ci) < 0.0)
      return ModBaseT< Mesh >::ILLEGAL_COLLAPSE;
  }
  return cmodule_->collapse_priority(_ci);
}

//-----------------------------------------------------------------------------

template<class Mesh>
void BaseDecimaterT<Mesh>::postprocess_collapse(CollapseInfo& _ci) {
  typename ModuleList::iterator m_it, m_end = bmodules_.end();

  for (m_it = bmodules_.begin(); m_it != m_end; ++m_it)
    (*m_it)->postprocess_collapse(_ci);

  cmodule_->postprocess_collapse(_ci);
}

//-----------------------------------------------------------------------------

template<class Mesh>
void BaseDecimaterT<Mesh>::preprocess_collapse(CollapseInfo& _ci) {
  typename ModuleList::iterator m_it, m_end = bmodules_.end();

  for (m_it = bmodules_.begin(); m_it != m_end; ++m_it)
    (*m_it)->preprocess_collapse(_ci);

  cmodule_->preprocess_collapse(_ci);
}

//-----------------------------------------------------------------------------

template<class Mesh>
void BaseDecimaterT<Mesh>::set_error_tolerance_factor(double _factor) {
  if (_factor >= 0.0 && _factor <= 1.0) {
    typename ModuleList::iterator m_it, m_end = bmodules_.end();

    for (m_it = bmodules_.begin(); m_it != m_end; ++m_it)
      (*m_it)->set_error_tolerance_factor(_factor);

    cmodule_->set_error_tolerance_factor(_factor);
  }
}

//-----------------------------------------------------------------------------

template<class Mesh>
void BaseDecimaterT<Mesh>::info(std::ostream& _os) {
  if (initialized_) {
    _os << "initialized : yes" << std::endl;
    _os << "binary modules: " << bmodules_.size() << std::endl;
    for (ModuleListIterator m_it = bmodules_.begin(); m_it != bmodules_.end();
        ++m_it) {
      _os << "  " << (*m_it)->name() << std::endl;
    }
    _os << "priority module: " << cmodule_->name().c_str() << std::endl;
  } else {
    _os << "initialized : no" << std::endl;
    _os << "available modules: " << all_modules_.size() << std::endl;
    for (ModuleListIterator m_it = all_modules_.begin();
        m_it != all_modules_.end(); ++m_it) {
      _os << "  " << (*m_it)->name() << " : ";
      if ((*m_it)->is_binary()) {
        _os << "binary";
        if ((*m_it)->name() == "Quadric") {
          _os << " and priority (special treatment)";
        }
      } else {
        _os << "priority";
      }
      _os << std::endl;
    }
  }
}

//-----------------------------------------------------------------------------

template<class Mesh>
bool BaseDecimaterT<Mesh>::initialize() {
  if (initialized_) {
    return true;
  }

  // FIXME: quadric module shouldn't be treated specially.
  // Q: Why?
  // A: It isn't generic and breaks encapsulation. Also, using string
  // name comparison is not reliable, since you can't guarantee that
  // no one else will name their custom module "Quadric".
  // Q: What should be done instead?
  // A: ModBaseT API should support modules that can be both binary
  // and priority, or BETTER YET, let the DecimaterT API specify the
  // priority module explicitly.

  // find the priority module: either the only non-binary module in the list, or "Quadric"
  Module *quadric = NULL;
  Module *pmodule = NULL;
  for (ModuleListIterator m_it = all_modules_.begin(), m_end =
      all_modules_.end(); m_it != m_end; ++m_it) {
    if ((*m_it)->name() == "Quadric")
      quadric = *m_it;

    if (!(*m_it)->is_binary()) {
      if (pmodule) {
        // only one priority module allowed!
        set_uninitialized();
        return false;
      }
      pmodule = *m_it;
    }
  }

  // Quadric is used as default priority module (even if it is set to be binary)
  if (!pmodule && quadric) {
    pmodule = quadric;
  }

  if (!pmodule) {
    // At least one priority module required
    set_uninitialized();
    return false;
  }

  // set pmodule as the current priority module
  cmodule_ = pmodule;

  for (ModuleListIterator m_it = all_modules_.begin(), m_end =
      all_modules_.end(); m_it != m_end; ++m_it) {
    // every module gets initialized
    (*m_it)->initialize();

    if (*m_it != pmodule) {
      // all other modules are binary, and go into bmodules_ list
      bmodules_.push_back(*m_it);
    }
  }

  return initialized_ = true;
}



//=============================================================================
}// END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================

